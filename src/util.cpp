#include "stdafx.h"
#include "util.h"

static const char* __base_path = "./var";
static long int __id = 0;

void util::id_inc(void)
{
	__id++;
}

bool util::save_data(const char* filepath, const acl::string& data)
{
	acl_make_dirs(__base_path, 0755);

	acl::ofstream out;
	if (out.open_write(filepath) == false)
	{
		logger_error("open %s error %s",
				filepath, acl::last_serror());
		return false;
	}

	::chmod(filepath, 0644);

	if (data.empty())
		return true;

	if (out.write(data) < 0)
	{
		logger_error("write to %s error %s",
				filepath, acl::last_serror());
		return false;
	}

	return true;
}

bool util::save_req_hdr(const char* cmd, const acl::string& hdr)
{
	acl::string filepath;
	filepath.format("%s/%s_req_hdr_%ld.txt", __base_path, cmd, __id);
	return save_data(filepath, hdr);
}

bool util::save_req(const char* cmd, const char* ext, const acl::string& data)
{
	acl::string filepath;
	filepath.format("%s/%s_req_%ld.%s", __base_path, cmd, __id, ext);
	return save_data(filepath, data);
}

bool util::save_res_hdr(const char* cmd, const acl::string& hdr)
{
	acl::string filepath;
	filepath.format("%s/%s_res_hdr_%ld.txt", __base_path, cmd, __id);
	return save_data(filepath, hdr);
}

bool util::save_res(const char* cmd, const char* ext, const acl::string& data)
{
	acl::string filepath;
	filepath.format("%s/%s_res_%ld.%s", __base_path, cmd, __id, ext);
	return save_data(filepath, data);
}
