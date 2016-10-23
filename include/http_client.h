#pragma once

class http_client
{
public:
	http_client(acl::HttpServletRequest& req,
		acl::HttpServletResponse& res,
		acl::ostream& out);
	~http_client();

	bool http_request();

private:
	acl::HttpServletRequest& req_;
	acl::HttpServletResponse& res_;
	acl::ostream& out_;
	acl::string cmd_;

	bool connect_server(const acl::string& server_addr,
		acl::http_client& client);

	bool transfer_request(acl::http_client* front,
		acl::http_client* backend);
	bool transfer_response(acl::http_client* front,
		acl::http_client* backend);
};
