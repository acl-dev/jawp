#pragma once

class util
{
public:
	util() {}
	~util() {}

	static void id_inc(void);
	static bool save_req_hdr(const char* cmd, const acl::string& hdr);
	static bool save_req(const char* cmd, const char* ext,
			const acl::string& data);
	static bool save_res_hdr(const char* cmd, const acl::string& hdr);
	static bool save_res(const char* cmd, const char* ext,
			const acl::string& data);
	static bool save_data(const char* filepath,
			const acl::string& data);
};
