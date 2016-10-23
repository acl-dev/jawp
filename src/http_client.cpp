#include "stdafx.h"
#include <memory>
#include <iostream>
#include "master_service.h"
#include "util.h"
#include "http_client.h"

#define	DEBUG	100

http_client::http_client(acl::HttpServletRequest& req,
	acl::HttpServletResponse& res,
	acl::ostream& out)
: req_(req)
, res_(res)
, out_(out)
{
	const char* ptr = req.getParameter("Cmd");
	if (ptr && *ptr)
		cmd_.format("%s_%ld", ptr, time(NULL));
}

http_client::~http_client()
{
}

bool http_client::connect_server(const acl::string& server_addr,
	acl::http_client& client)
{
	// �Ȳ鱾��ӳ�������û��ӳ����
	//master_service& ms = acl::singleton2<master_service>::get_instance();
	//const char* addr = ms.get_map_addr(server_addr.c_str());
	const char* addr = 0;
	if (addr == NULL)
	{
		addr = server_addr.c_str();
		logger("addr: %s ---> direct access", addr);
	}
	else
		logger("addr: %s ---> addr: %s", server_addr.c_str(), addr);

	if (client.open(addr, 60, 60, true) == false)
	{
		out_.format("connect server %s error", addr);
		return false;
	}
	else
		logger_debug(DEBUG, 1, "connect server ok");

	return true;
}

bool http_client::http_request()
{
	acl::string server_addr;

	// ���Ȳ�������--��ַӳ�䷽ʽ����÷���˵�ַ
	const char* cmd = req_.getParameter("Cmd");
	if (cmd && *cmd)
	{
		//const char* addr = acl::singleton2<master_service>
		//	::get_instance().get_cmd_addr(cmd);
		const char* addr = NULL;
		if (addr && *addr)
		{
			server_addr = addr;
			logger("cmd: %s ---> addr: %s", cmd, addr);
		}
	}

	// ���δ��������Ӧ�ĺ�˵�ַ����� HTTP ����ͷ�л��
	if (server_addr.empty())
	{
		const char* host = req_.getRemoteHost();
		if (host == NULL || *host == 0)
			host = req_.getLocalAddr();

		if (strchr(host, ':') == NULL)
			server_addr.format("%s:80", host);
		else
			server_addr = host;
	}

	acl::http_client* backend = new acl::http_client;

	// ���ӷ�����
	if (connect_server(server_addr, *backend) == false)
	{
		delete backend;
		return false;
	}

	acl::http_client* front = req_.getClient();

	// ���ͻ�������ת���������
	if (transfer_request(front, backend) == false)
	{
		delete backend;
		return false;
	}

	// �����������ת�����ͻ���
	if (transfer_response(front, backend) == false)
	{
		delete backend;
		return false;
	}

	delete backend;
	return true;
}

bool http_client::transfer_request(acl::http_client* front,
	acl::http_client* backend)
{
	util::id_inc();

	// �ڽػ� hotmail ����ʱ����Ҫǿ������ Host �ֶΣ����� hotmail
	// �����Ҫ��ͻ����ض����ƹ������������
	//if (var_cfg_set_host && *var_cfg_set_host)
	//	front->header_update("Host", var_cfg_set_host, true);
		//front->header_update("Host", "blu405-m.hotmail.com", true);

	// ȡ��  HTTP ����ͷ����
	acl::string req_hdr;
	front->sprint_header(req_hdr, NULL);

	util::save_req_hdr(cmd_, req_hdr);

	// ת�� HTTP ����ͷ��������
	if (backend->get_ostream().write(req_hdr) == -1)
	{
		out_.puts(">>>>write header error");
		return false;
	}
	if (backend->get_ostream().write("\r\n") == -1)
	{
		out_.puts(">>>>write CRLF error");
		return false;
	}

	bool convert = false;
	size_t length = 8129;

	std::shared_ptr<acl::string> body(new acl::string(length));
	const char* stype;
	acl::http_ctype http_ctype;
	const char* ptr = front->header_value("Content-Type");

	if (ptr && *ptr && http_ctype.parse(ptr) == true
		&& (stype = (http_ctype.get_stype())) != NULL
		&& strcasecmp(stype, "vnd.ms-sync.wbxml") == 0)
	{
		convert = true;
	}

	printf(">>>request content-type: %s, convert: %s\r\n",
		ptr ? ptr : "NULL", convert ? "true" : "false");

	// ������������壬��ת��������������������
	long long int len = req_.getContentLength();

	const char* ext = "unknown";
	acl::string saved_buf;

	if (len > 0)
	{
		char req_body[8192];
		int ret;

		while (true)
		{
			ret = front->read_body(req_body, sizeof(req_body) - 1);
			if (ret < 0)
			{
				out_.puts(">>> read req body error");
				return false;
			}
			if (ret == 0)
				break;

			body->append(req_body, ret);

			if (backend->get_ostream().write(req_body, ret) == -1)
			{
				out_.puts(">>> write body to server error");
				return false;
			}
		}

		out_.puts(">>>> request <<<<");
		out_.write(*body);
		saved_buf = *body;
		ext = "txt";
	}

	out_.puts("");

	// ���ͻ������󱣴�Ϊ�������ļ�
	util::save_req(cmd_, ext, saved_buf);

	return true;
}

bool http_client::transfer_response(acl::http_client*,
	acl::http_client* backend)
{
	// ��ʼ�Ӻ�˷�������ȡ��Ӧͷ����Ӧ������

	out_.puts(">>>> begin read res header<<<<<");
	if (backend->read_head() == false)
	{
		out_.puts(">>>>>>>>read header error<<<<<<<<<<");
		return false;
	}

	acl::string res_hdr;
	backend->sprint_header(res_hdr, NULL);

	util::save_res_hdr(cmd_, res_hdr);

	if (res_.getOutputStream().write(res_hdr) == -1)
	{
		out_.puts(">>>>>write res hdr error<<<<<<");
		return false;
	}

	if (res_.getOutputStream().write("\r\n") == -1)
	{
		out_.puts(">>>write CRLF error");
		return false;
	}

	out_.puts("------------------res hdr----------------");
	out_.write(res_hdr);
	out_.puts("------------------res hdr end------------");

	bool convert = false;
	acl::string body;

	const char* ptr = backend->header_value("Content-Type");
	acl::http_ctype http_ctype;
	const char* stype;

	if (ptr && *ptr && http_ctype.parse(ptr) == true
		&& (stype = (http_ctype.get_stype())) != NULL
		&& strcasecmp(stype, "vnd.ms-sync.wbxml") == 0)
	{
		convert = true;
	}


	long long length = backend->body_length();

	printf(">>>response content-type: %s, convert: %s, content-length: %lld\r\n",
		ptr ? ptr : "NULL", convert ? "true" : "false", length);

	if (length <= 0)
		return true;

	const char* ext = "unknown";
	acl::string saved_buf;

	char buf[8192];

	while (true)
	{
		int ret = backend->read_body(buf, sizeof(buf) - 1);
		if (ret < 0)
		{
			logger_error(">>> read body error");
			return false;
		}
		else if (ret == 0)
			break;

		body.append(buf, ret);

		if (res_.getOutputStream().write(buf, ret) == -1)
		{
			out_.puts(">>> write res body error");
			return false;
		}
	}

	out_.puts(">>>> response <<<<");
	out_.write(body);
	saved_buf = body;
	ext = "txt";

	util::save_res(cmd_, ext, saved_buf);

	ptr = backend->header_value("Transfer-Encoding");
	if (ptr == NULL || *ptr == 0 || strcasecmp(ptr, "chunked") != 0)
		return backend->keep_alive();

	// ���� http ��Ӧ�壬��Ϊ������ chunk ����ģʽ��������Ҫ�����һ��
	// res.write ������������Ϊ 0 �Ա�ʾ chunk �������ݽ���
	return res_.write(NULL, 0);
}
