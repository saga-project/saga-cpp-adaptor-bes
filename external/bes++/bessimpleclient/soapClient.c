/* soapClient.c
   Generated by gSOAP 2.7.10 from bes-factory.h
   Copyright(C) 2000-2008, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/
#include "soapH.h"
#ifdef __cplusplus
extern "C" {
#endif

SOAP_SOURCE_STAMP("@(#) soapClient.c ver 2.7.10 2010-03-26 09:24:04 GMT")


SOAP_FMAC5 int SOAP_FMAC6 soap_call___bes__CreateActivity(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct bes__CreateActivityType *bes__CreateActivity, struct bes__CreateActivityResponseType *bes__CreateActivityResponse)
{	struct __bes__CreateActivity soap_tmp___bes__CreateActivity;
	if (!soap_action)
		soap_action = "http://schemas.ggf.org/bes/2006/08/bes-factory/BESFactoryPortType/CreateActivity";
	soap->encodingStyle = NULL;
	soap_tmp___bes__CreateActivity.bes__CreateActivity = bes__CreateActivity;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___bes__CreateActivity(soap, &soap_tmp___bes__CreateActivity);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___bes__CreateActivity(soap, &soap_tmp___bes__CreateActivity, "-bes:CreateActivity", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___bes__CreateActivity(soap, &soap_tmp___bes__CreateActivity, "-bes:CreateActivity", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	soap_default_bes__CreateActivityResponseType(soap, bes__CreateActivityResponse);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get_bes__CreateActivityResponseType(soap, bes__CreateActivityResponse, "bes:CreateActivityResponse", "bes:CreateActivityResponseType");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___bes__GetActivityStatuses(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct bes__GetActivityStatusesType bes__GetActivityStatuses, struct bes__GetActivityStatusesResponseType *bes__GetActivityStatusesResponse)
{	struct __bes__GetActivityStatuses soap_tmp___bes__GetActivityStatuses;
	if (!soap_action)
		soap_action = "http://schemas.ggf.org/bes/2006/08/bes-factory/BESFactoryPortType/GetActivityStatuses";
	soap->encodingStyle = NULL;
	soap_tmp___bes__GetActivityStatuses.bes__GetActivityStatuses = bes__GetActivityStatuses;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___bes__GetActivityStatuses(soap, &soap_tmp___bes__GetActivityStatuses);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___bes__GetActivityStatuses(soap, &soap_tmp___bes__GetActivityStatuses, "-bes:GetActivityStatuses", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___bes__GetActivityStatuses(soap, &soap_tmp___bes__GetActivityStatuses, "-bes:GetActivityStatuses", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	soap_default_bes__GetActivityStatusesResponseType(soap, bes__GetActivityStatusesResponse);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get_bes__GetActivityStatusesResponseType(soap, bes__GetActivityStatusesResponse, "bes:GetActivityStatusesResponse", "bes:GetActivityStatusesResponseType");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___bes__TerminateActivities(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct bes__TerminateActivitiesType bes__TerminateActivities, struct bes__TerminateActivitiesResponseType *bes__TerminateActivitiesResponse)
{	struct __bes__TerminateActivities soap_tmp___bes__TerminateActivities;
	if (!soap_action)
		soap_action = "http://schemas.ggf.org/bes/2006/08/bes-factory/BESFactoryPortType/TerminateActivities";
	soap->encodingStyle = NULL;
	soap_tmp___bes__TerminateActivities.bes__TerminateActivities = bes__TerminateActivities;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___bes__TerminateActivities(soap, &soap_tmp___bes__TerminateActivities);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___bes__TerminateActivities(soap, &soap_tmp___bes__TerminateActivities, "-bes:TerminateActivities", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___bes__TerminateActivities(soap, &soap_tmp___bes__TerminateActivities, "-bes:TerminateActivities", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	soap_default_bes__TerminateActivitiesResponseType(soap, bes__TerminateActivitiesResponse);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get_bes__TerminateActivitiesResponseType(soap, bes__TerminateActivitiesResponse, "bes:TerminateActivitiesResponse", "bes:TerminateActivitiesResponseType");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___bes__GetActivityDocuments(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct bes__GetActivityDocumentsType bes__GetActivityDocuments, struct bes__GetActivityDocumentsResponseType *bes__GetActivityDocumentsResponse)
{	struct __bes__GetActivityDocuments soap_tmp___bes__GetActivityDocuments;
	if (!soap_action)
		soap_action = "http://schemas.ggf.org/bes/2006/08/bes-factory/BESFactoryPortType/GetActivityDocuments";
	soap->encodingStyle = NULL;
	soap_tmp___bes__GetActivityDocuments.bes__GetActivityDocuments = bes__GetActivityDocuments;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___bes__GetActivityDocuments(soap, &soap_tmp___bes__GetActivityDocuments);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___bes__GetActivityDocuments(soap, &soap_tmp___bes__GetActivityDocuments, "-bes:GetActivityDocuments", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___bes__GetActivityDocuments(soap, &soap_tmp___bes__GetActivityDocuments, "-bes:GetActivityDocuments", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	soap_default_bes__GetActivityDocumentsResponseType(soap, bes__GetActivityDocumentsResponse);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get_bes__GetActivityDocumentsResponseType(soap, bes__GetActivityDocumentsResponse, "bes:GetActivityDocumentsResponse", "bes:GetActivityDocumentsResponseType");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___bes__GetFactoryAttributesDocument(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct bes__GetFactoryAttributesDocumentType *bes__GetFactoryAttributesDocument, struct bes__GetFactoryAttributesDocumentResponseType *bes__GetFactoryAttributesDocumentResponse)
{	struct __bes__GetFactoryAttributesDocument soap_tmp___bes__GetFactoryAttributesDocument;
	if (!soap_action)
		soap_action = "http://schemas.ggf.org/bes/2006/08/bes-factory/BESFactoryPortType/GetFactoryAttributesDocument";
	soap->encodingStyle = NULL;
	soap_tmp___bes__GetFactoryAttributesDocument.bes__GetFactoryAttributesDocument = bes__GetFactoryAttributesDocument;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___bes__GetFactoryAttributesDocument(soap, &soap_tmp___bes__GetFactoryAttributesDocument);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___bes__GetFactoryAttributesDocument(soap, &soap_tmp___bes__GetFactoryAttributesDocument, "-bes:GetFactoryAttributesDocument", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___bes__GetFactoryAttributesDocument(soap, &soap_tmp___bes__GetFactoryAttributesDocument, "-bes:GetFactoryAttributesDocument", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	soap_default_bes__GetFactoryAttributesDocumentResponseType(soap, bes__GetFactoryAttributesDocumentResponse);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get_bes__GetFactoryAttributesDocumentResponseType(soap, bes__GetFactoryAttributesDocumentResponse, "bes:GetFactoryAttributesDocumentResponse", "bes:GetFactoryAttributesDocumentResponseType");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

#ifdef __cplusplus
}
#endif

/* End of soapClient.c */
