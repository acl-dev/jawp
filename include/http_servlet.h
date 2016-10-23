#pragma once

class http_servlet : public acl::HttpServlet
{
public:
	http_servlet(acl::ostream& out);
	~http_servlet();

protected:
	//@override
	bool doGet(acl::HttpServletRequest&, acl::HttpServletResponse&);

	//@override
	bool doPost(acl::HttpServletRequest&, acl::HttpServletResponse&);

	//@override
	bool doUnknown(acl::HttpServletRequest&, acl::HttpServletResponse&);

	//@override
	bool doError(acl::HttpServletRequest&, acl::HttpServletResponse&);

	//@override
	bool doConnect(acl::HttpServletRequest&, acl::HttpServletResponse&);

	//@override
	bool doPut(acl::HttpServletRequest&, acl::HttpServletResponse&);

	//@override
	bool doDelete(acl::HttpServletRequest&, acl::HttpServletResponse&);

	//@override
	bool doHead(acl::HttpServletRequest&, acl::HttpServletResponse&);

	//@override
	bool doOptions(acl::HttpServletRequest&, acl::HttpServletResponse&);

private:
	bool handled_;
	acl::string url_;
	acl::ostream& out_;
	acl::polarssl_conf* client_ssl_conf_;

	void logger_request(acl::HttpServletRequest& req);
};
