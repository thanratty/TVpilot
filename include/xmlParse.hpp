#pragma once

#include "Cshow.hpp"
#include "CcurlJob.hpp"



constexpr int E_XPARSE_OK			= 0;
constexpr int E_XPARSE_DOC_ERROR	= 1;
constexpr int E_XPARSE_PAGE_ERROR	= 2;




typedef struct xml_error_info_tag
{
	int                 xmlErrorCode{ INT_MAX };
	//
	int                 xmlErrorDomain;		// These four vars are meaningful if xmlErrorCode isn't XML_ERR_NONE
	int                 xmlErrorLine;
	int                 xmlErrorCol;
	std::string         xmlErrorMsg;
} sXmlErrorInfo;



int xmlParse(show& show, const cCurlJob& curljob, sXmlErrorInfo& xml_error_info);
