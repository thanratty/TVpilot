#include "config.h"

#pragma warning( disable : 26812 )
#include "pch.h"

//--


#include <vector>
#include <regex>

#include "boost/algorithm/string/trim.hpp"

#include "libxml/xpath.h"
#include "libxml/HTMLparser.h"
#include "libxml/HTMLtree.h"

#include "Cepisode.h"
#include "Cshow.h"
#include "CcurlJob.hpp"
#include "utils.hpp"

#include "xmlParse.hpp"




STATIC const char* DEFAULT_EPISODE_DATE   = "1900 Jan 01";
STATIC const char* DEFAULT_EPISODE_NUMBER = "00-00";




/**
 * Locally used structure only
 */
typedef struct MyXpathResults_tag
{
	std::vector<std::string> epnum;
	std::vector<std::string> epdate;
	std::vector<std::string> eptitle;
} sMyXpathResults;



// For convenience
typedef const xmlChar* PXMLCHAR;


// UTF-8 encoding for HTML &nbsp character
STATIC const char* chNBSP = "\xC2\xA0";




STATIC
bool my_replace(std::string& str, const std::string& from, const std::string& to)
{
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}




STATIC
std::string GuessTvdbUrl(const std::string& title)
{
	std::string		url("https://thetvdb.com/series/");
	std::string		show(title);

	for (auto& ch : show)
		if (! isalnum(ch))
			ch = '-';

	// Remove all double dashes
	while(my_replace(show, "--", "-"));

	// Remove any trailing dash
	if (show[show.length()-1] == '-')
		show.pop_back();

	url += show;
	return url;
}











/**
 * Run the given XPATH query and return the result nodes
 */
STATIC
xmlXPathObjectPtr runXquery(htmlDocPtr document, PXMLCHAR xpath_query_string)
{
	xmlXPathContextPtr xpath_ctx = xmlXPathNewContext(document);
	xmlXPathObjectPtr  nodes     = xmlXPathEvalExpression(xpath_query_string, xpath_ctx);

	xmlXPathFreeContext(xpath_ctx);
	return nodes;
}



/**
 * Extract the inner text from a node pointer from the XPATH query results set
 */
STATIC
std::string getNodeText(const xmlNodePtr cur)
{
	std::string content;

	if ((cur->children != NULL) && (cur->children[0].type == XML_TEXT_NODE))
	{
		content = std::string((LPCSTR)cur->children[0].content);
		// Replace all UTF-8 &nbsp (hex C2 A0) characters with ASCII spaces
		ReplaceAllSubstrings(content, chNBSP);
		boost::trim(content);
	}

	return content;
}




STATIC
std::string extractUniqueNode(xmlXPathObjectPtr nodes)
{
	std::string str;

	xmlNodeSetPtr nodeset = nodes->nodesetval;
	if (nodeset != nullptr)
	{
		int size = nodeset->nodeNr;
		if (size != 1)
			WriteMessageLog(L"ERROR! XPATH query returned more than one node");

		// The webpage should have exactly one element
		if ((nodeset->nodeTab[0]->type == XML_ELEMENT_NODE) || (nodeset->nodeTab[0]->type == XML_ATTRIBUTE_NODE))
			str = getNodeText(nodeset->nodeTab[0]);
		else
			WriteMessageLog(L"ERROR! Unexpected XML node type.");
	}

	return str;
}






/**
 * Extract all epsiode numbers and air dates from the XPATH query results set and saves them in our local results vectors
 * 
 * NB Fixes invalid date or episode number values
 */
STATIC
bool extractEpisodeDetails( xmlXPathObjectPtr nodes, sMyXpathResults& results )
{
	xmlNodeSetPtr	nodeset = nodes->nodesetval;

	// Sanity check on results set
	if ((nodeset == nullptr) || (nodeset->nodeNr == 0))
	{
		WriteMessageLog("extractEpisodeDetails() Error. Empty results set!");
		return false;
	}

	const std::regex epdate_regex("^\\ *[0-9]{2}\\ [A-Z][a-z]{2}\\ [0-9]{2}\\ *");
	const std::regex epnum_regex("^\\ *[1-9]{1}[0-9]?-[0-9]{1,2}\\ *");


	int size = nodeset->nodeNr;
	std::string ep_number, ep_date, ep_date_fixed;

	for (int i = 0; i < size; i += 3)
	{
		// The 1st entry in the results at index [i+0] is the episode entry # on the web page. We don't need that.
		ep_number = getNodeText(nodeset->nodeTab[i + 1]);
		ep_date   = getNodeText(nodeset->nodeTab[i + 2]);

		bool bGoodDate = std::regex_match(ep_date, epdate_regex);
		bool bGoodNumber = std::regex_match(ep_number, epnum_regex);

		// If they're both bad - just skip this node. A few epguides.com pages had different formatting.
		if (!bGoodDate || !bGoodNumber)
			continue;

		// Validate the date format & fix if necessary
		if (!bGoodDate)
		{
			std::string msg = "extractEpisodeDetails() Bad episode date format. Defaulted. [" + ep_date + "]";
			WriteMessageLog(msg);
			ep_date_fixed = DEFAULT_EPISODE_DATE;
		}
		else
		{
			// Dates come in from epguides formatted as (eg) '27 Oct 14'. It needs the century digits adding & reformatting to YYYY MMM DD
			int year = std::stoi(ep_date.substr(7));
			ep_date.insert(7, ((year == 0) || (year < 40)) ? "20" : "19");
			ep_date_fixed = ep_date.substr(7, 4) + ep_date.substr(2, 5) + ep_date.substr(0, 2);
		}


		// Validate the episode number format & fix if necessary
		if (!bGoodNumber)
		{
			std::string msg = "extractEpisodeDetails() Bad episode number format. Defaulted. [" + ep_number + "]";
			WriteMessageLog(msg);
			ep_number = DEFAULT_EPISODE_NUMBER;
		}


		// Add episode number & airdate to the local results set
		results.epnum.push_back(ep_number);
		results.epdate.push_back(ep_date_fixed);

	}


	return true;
}




STATIC
bool extractEpisodeTitles( xmlXPathObjectPtr nodes, sMyXpathResults& results)
{
	bool retval = false;
	xmlNodeSetPtr nodeset = nodes->nodesetval;
	if (nodeset != nullptr)
	{
		int size = nodeset->nodeNr;

		for (int i = 0; i < size; i++)
		{
			std::string title = getNodeText(nodeset->nodeTab[i]);
			results.eptitle.push_back(title);
		}

		retval = true;
	}

	return retval;
}




/**********************************************************************************
 * 
 * E_XPARSE_OK			Success
 * E_XPARSE_DOC_ERROR	
 * Anything else = XML parse error
 *
 **********************************************************************************/

int xmlParse( show& show, const cCurlJob& curljob, sXmlErrorInfo& xml_error_info )
{
	// Presume success
	int retval = E_XPARSE_OK;
	xml_error_info.xmlErrorCode = XML_ERR_NONE;

	// curljob is the master data here. Setup the empty show object.
	show.title        = curljob.Url();
	show.epguides_url = curljob.Url();
	show.hash         = SimpleHash(curljob.Url());


	// Create a thread-safe parser context
	htmlParserCtxtPtr context = htmlNewParserCtxt();

	// Parse the web page into an HTML DOM
	htmlDocPtr doc = htmlCtxtReadMemory(context,
										curljob.HtmlData(),
										curljob.HtmlLength(),
										NULL,
										"UTF-8",
										HTML_PARSE_NOWARNING | HTML_PARSE_NOERROR | HTML_PARSE_RECOVER | HTML_PARSE_NODEFDTD );

	if (doc == NULL)
	{
		// Something went wrong. Setup return codes etc.
		xmlErrorPtr perror = xmlCtxtGetLastError(context);
		if (perror != nullptr)
		{
			xml_error_info.xmlErrorCode	  = perror->code;
			xml_error_info.xmlErrorDomain = perror->domain;
			xml_error_info.xmlErrorMsg    = perror->message;
			xml_error_info.xmlErrorLine   = perror->line;
			xml_error_info.xmlErrorCol    = perror->int2;
		}
		xmlCtxtResetLastError(context);
		retval = E_XPARSE_DOC_ERROR;
	}
	else
	{
		xmlXPathObjectPtr showTitleNode      = runXquery(doc, (PXMLCHAR) "//h2/a");
		xmlXPathObjectPtr showIMDBurl        = runXquery(doc, (PXMLCHAR) "//div[@class='IMDB']/a/@href");
		xmlXPathObjectPtr showTVMazeurl      = runXquery(doc, (PXMLCHAR) "//div[@class='TVMaze']/*[text()[contains(.,'Show Summary')]]/@href");
		xmlXPathObjectPtr episodeTitleNodes  = runXquery(doc, (PXMLCHAR) "//td[contains(@class,'eptitle')]/a");
		xmlXPathObjectPtr episodeDetailNodes = runXquery(doc, (PXMLCHAR) "//td[contains(@class,'epinfo')]");

		sMyXpathResults   xpath_results;

		show.title       = extractUniqueNode(showTitleNode);
		show.imdb_url    = extractUniqueNode(showIMDBurl);
		show.tvmaze_url  = extractUniqueNode(showTVMazeurl);
		show.thetvdb_url = GuessTvdbUrl(show.title);
		bool bGotTitles  = extractEpisodeTitles(episodeTitleNodes, xpath_results) && (xpath_results.eptitle.size()>0);
		bool bGotDetails = extractEpisodeDetails(episodeDetailNodes, xpath_results) && (xpath_results.epnum.size()>0) && (xpath_results.epdate.size()>0);

		xmlXPathFreeObject(showTitleNode);
		xmlXPathFreeObject(showIMDBurl);
		xmlXPathFreeObject(showTVMazeurl);
		xmlXPathFreeObject(episodeTitleNodes);
		xmlXPathFreeObject(episodeDetailNodes);

		// Make sure everything looks valid
		if (!bGotTitles || !bGotDetails || (show.title.length() == 0))
		{
			show.state |= showstate::SH_ST_UPDATE_FAILED;
			retval = E_XPARSE_PAGE_ERROR;
		}
		else
		{
			for (unsigned i = 0; i < xpath_results.epnum.size(); i++)
			{
				episode ep;
				ep.ep_title       = xpath_results.eptitle[i];
				ep.ep_num         = xpath_results.epnum[i];
				ep.ep_date_string = xpath_results.epdate[i];
				ep.ep_date        = gregorian::from_simple_string(ep.ep_date_string);

				// Add(copy) this local episode to the supplied show's episode list
				show.state = showstate::SH_ST_UPDATED;
				show.episodes.push_back(ep);
			}
		}
	}

	htmlFreeParserCtxt(context);
	xmlFreeDoc(doc);

	return retval;
}

