#pragma once

#include "Cshow.hpp"
#include "CcurlJob.hpp"



constexpr int E_XPARSE_OK					= 0;
constexpr int E_XPARSE_DOC_FORMAT_ERROR		= 1;
constexpr int E_XPARSE_PAGE_FORMAT_ERROR	= 2;




typedef struct xml_error_info_tag
{
	int                 xmlErrorCode{ INT_MAX };
	//
	int                 xmlErrorDomain{ 0 };		// These four vars are meaningful if xmlErrorCode isn't XML_ERR_NONE
	int                 xmlErrorLine{ 0 };
	int                 xmlErrorCol{ 0 };
	std::string         xmlErrorMsg;
} sXmlErrorInfo;



int xmlParse(show& show, const cCurlJob& curljob, sXmlErrorInfo& xml_error_info);
