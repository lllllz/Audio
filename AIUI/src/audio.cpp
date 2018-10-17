/*******************************************************
* File:			audio.cpp
* Date:			2016-10-24
* Author:		lllllz
* Description:	实现AIUI语音模块的串口功能
*******************************************************/

/******************************************************* 
* Function List: 
* sigHandler		信号处理函数
* getId				获取消息ID
* calcCheckCode		计算校验位
* textToSpeech		串口语音合成
* openVoice			打开播报
* closeVoice		关闭播报
* configureWifi		配置wifi设置
* getWifiStatus		获取wifi连接状态
* sendMsg			发送消息
* gzUncompress		解压gzip压缩后的数据
* getTimeStatus		打印当前时间
* threadTTS			语音合成线程函数
* processRecv		分析接收到的串口消息
* uartRec			接收串口消息函数
*******************************************************/

#include "../include/audio.hpp"

/* ------------------------------------------------------------------------
** Function Implementations
** ------------------------------------------------------------------------ */
/*******************************************************
* 函数名:			sigHandler
* 函数说明:		信号处理函数
* 参数:
*	signo 		信号
* 返回值:			无
*******************************************************/
void sigHandler(int signo)
{
	g_quit = 1;
}

/*******************************************************
* 函数名:			getId
* 函数说明:		获取消息ID
* 参数:
*	select 		指定高/低字节消息ID的获取 1 - 低字节消息ID 2 - 高字节消息ID
* 返回值:			无
*******************************************************/
int getId(int select)
{
	if (1 == select)
	{
		if (msg_count == 255)
		{
			msg_high_id += 1;
			msg_low_id = 0;
			msg_count = 0;
		}
		msg_count += 1;
		return msg_low_id + msg_count;
	}
	else if (2 == select)
		return msg_high_id;
}

/*******************************************************
* 函数名:			calcCheckCode
* 函数说明:		计算校验位
* 参数:
*	buf 				需要进行计算的buf
*	length 		buf长度
* 返回值:		计算得到校验位
*******************************************************/
unsigned char calcCheckCode(unsigned char* buf, int length)
{
	int i = 0;
	// 计算校检码
	unsigned char check_code = 0;
	for (i = 0; i < length; i++)
	{
		check_code += buf[i];
	}
	check_code = ~check_code + 1;
	return check_code;
}

/*******************************************************
* 函数名:			calcCheckCode
* 函数说明:		计算校验位
* 参数:
*	text 				需要进行合成的文字
* 返回值:		计算得到校验位
*******************************************************/
void textToSpeech(const char* text)
{
	// 串口合成
    int i = 0;
	char* c_tts_json;
	cJSON* p_tts_json1, * p_tts_json2;
	p_tts_json1 = cJSON_CreateObject();	// 创建项目
	cJSON_AddItemToObject(p_tts_json1, "type", cJSON_CreateString("tts"));
	cJSON_AddItemToObject(p_tts_json1, "content", p_tts_json2 = cJSON_CreateObject());	// 在项目上添加项目
	// 在项目上的项目上添加字符串，这说明cJSON是可以嵌套的
	cJSON_AddStringToObject(p_tts_json2, "action", "start");
	cJSON_AddStringToObject(p_tts_json2, "text", text);

	c_tts_json = cJSON_Print(p_tts_json1);
	printf("c_tts_json:\n%s\n", c_tts_json);	// 此时out指向的字符串就是JSON格式的了

	unsigned char c_tts[7 + strlen(c_tts_json) + 1];
	memset(c_tts, 0, 7 + strlen(c_tts_json) + 1);
	c_tts[0] = 0xA5;
	c_tts[1] = 0x01;
	c_tts[2] = 0x05;
	//sprintf(&c_tts[3], "%02X", (unsigned char)strlen(c_tts_json));
	c_tts[3] = strlen(c_tts_json);
	c_tts[4] = 0x00;
	c_tts[5] = getId(1);
	c_tts[6] = getId(2);
	for (i = 0; i < strlen(c_tts_json); i++)
		c_tts[7 + i] = c_tts_json[i];

	c_tts[7 + strlen(c_tts_json)] = calcCheckCode(c_tts, 7 + strlen(c_tts_json));

	uartSend(uart_hd, c_tts, 7 + strlen(c_tts_json) + 1);

	cJSON_Delete(p_tts_json1);
	free(c_tts_json);
}

/*******************************************************
* 函数名:			openVoice
* 函数说明:		打开播报
* 参数:
* 返回值:			无
*******************************************************/
void openVoice()
{
	int i = 0;
	// 打开播报
	char* c_open_voice_json;
	cJSON* p_open_voice_json1,* p_open_voice_json2;
	p_open_voice_json1 = cJSON_CreateObject();	// 创建项目
	cJSON_AddItemToObject(p_open_voice_json1, "type", cJSON_CreateString("voice"));
	cJSON_AddItemToObject(p_open_voice_json1, "content", p_open_voice_json2 = cJSON_CreateObject());	// 在项目上添加项目
	// 在项目上的项目上添加字符串，这说明cJSON是可以嵌套的
	cJSON_AddStringToObject(p_open_voice_json2, "enable_voice", "true");

	c_open_voice_json = cJSON_Print(p_open_voice_json1);
	printf("c_open_voice_json:\n%s\n", c_open_voice_json);	// 此时out指向的字符串就是JSON格式的了

	unsigned char c_open_voice[7 + strlen(c_open_voice_json) + 1];
	memset(c_open_voice, 0, 7 + strlen(c_open_voice_json) + 1);
	c_open_voice[0] = 0xA5;
	c_open_voice[1] = 0x01;
	c_open_voice[2] = 0x05;
	c_open_voice[3] = strlen(c_open_voice_json);
	c_open_voice[4] = 0x00;
	c_open_voice[5] = getId(1);
	c_open_voice[6] = getId(2);
	for (i = 0; i < strlen(c_open_voice_json); i++)
		c_open_voice[7 + i] = c_open_voice_json[i];

	char openvoice_code = 0;
	for (i = 0; i <= 7 + strlen(c_open_voice_json) - 1; i++)
	{
		openvoice_code += c_open_voice[i];
	}
	openvoice_code = ~openvoice_code + 1;
	c_open_voice[7 + strlen(c_open_voice_json)] = calcCheckCode(c_open_voice, 7 + strlen(c_open_voice_json));

	uartSend(uart_hd, c_open_voice, 7 + strlen(c_open_voice_json) + 1);

	cJSON_Delete(p_open_voice_json1);
	free(c_open_voice_json);
}

/*******************************************************
* 函数名:			closeVoice
* 函数说明:		关闭播报
* 参数:
* 返回值:			无
*******************************************************/
void closeVoice()
{
	int i = 0;
	// 关闭播报
	char* c_close_voice_json;
	cJSON* p_close_voice_json1, * p_close_voice_json2;
	p_close_voice_json1 = cJSON_CreateObject();	// 创建项目
	cJSON_AddItemToObject(p_close_voice_json1, "type", cJSON_CreateString("voice"));
	cJSON_AddItemToObject(p_close_voice_json1, "content", p_close_voice_json2 = cJSON_CreateObject());	// 在项目上添加项目
	// 在项目上的项目上添加字符串，这说明cJSON是可以嵌套的
	cJSON_AddStringToObject(p_close_voice_json2, "enable_voice", "false");

	c_close_voice_json = cJSON_Print(p_close_voice_json1);
	printf("c_close_voice_json:\n%s\n", c_close_voice_json);	 // 此时out指向的字符串就是JSON格式的了

	unsigned char c_close_voice[7 + strlen(c_close_voice_json) + 1];
	memset(c_close_voice, 0, 7 + strlen(c_close_voice_json) + 1);
	c_close_voice[0] = 0xA5;
	c_close_voice[1] = 0x01;
	c_close_voice[2] = 0x05;
	//sprintf(&c_close_voice[3], "%02X", (unsigned char)strlen(c_close_voice_json));
	c_close_voice[3] = strlen(c_close_voice_json);
	c_close_voice[4] = 0x00;
	c_close_voice[5] = getId(1);
	c_close_voice[6] = getId(2);
	for (i = 0; i < strlen(c_close_voice_json); i++)
		c_close_voice[7 + i] = c_close_voice_json[i];

	c_close_voice[7 + strlen(c_close_voice_json)] = calcCheckCode(c_close_voice, 7 + strlen(c_close_voice_json));

	uartSend(uart_hd, c_close_voice, 7 + strlen(c_close_voice_json) + 1);

	cJSON_Delete(p_close_voice_json1);
	free(c_close_voice_json);
}

/*******************************************************
* 函数名:			configureWifi
* 函数说明:		配置wifi设置
* 参数:
*	filename	配置文件路径
* 返回值:			无
*******************************************************/
void configureWifi(const char* filename)
{
	int i = 0;
	// fopen打开配置文件读取wifi配置信息
	FILE* fp = fopen(filename, "r");
	if (NULL != fp)
	{
		printf("[configureWifi] param file open successful\n");
		fseek(fp, 0, SEEK_END); // 定位到文件末
		int n_file_length = ftell(fp); // 获取文件长度 
		printf("[configureWifi] param json length:%d\n", n_file_length);
		char buf[n_file_length];
		if (n_file_length > 0)
		{
			fseek(fp, 0, SEEK_SET);	// 定位到文件首
			fread(buf, n_file_length, 1, fp);
			printf("[configureWifi] param buf:\n%s\n", buf);

			cJSON* p_json = cJSON_Parse((char*)buf);
			if (NULL != p_json)
			{	
				// wifi配置
				cJSON* p_wifi = cJSON_GetObjectItem(p_json, "wifi");
				cJSON* p_encryption = cJSON_GetObjectItem(p_wifi, "way_of_encryption");
				cJSON* p_name = cJSON_GetObjectItem(p_wifi, "name");
				cJSON* p_password = cJSON_GetObjectItem(p_wifi, "password");
				if ((0 == p_encryption->valueint) && (NULL != p_name->valuestring))
				{
					printf("[configureWifi] p_encryption:%d p_name:%s\n", p_encryption->valueint, p_name->valuestring);
				}
				else if ((NULL != p_name->valuestring))
				{
					if (NULL != p_password->valuestring)
					{
						printf("[configureWifi] p_encryption:%d p_name:%s p_password:%s\n", p_encryption->valueint, p_name->valuestring, p_password->valuestring);
					}
				}
				int n_name = strlen(p_name->valuestring); // 计算wifi名称的长度
				int n_password = strlen(p_password->valuestring); // 计算wifi密码的长度
				unsigned char wifi_info[9 + n_name + n_password + 1];
				memset(wifi_info, 0, 9 + n_name + n_password + 1);
				wifi_info[0] = 0xA5;													// 同步头
				wifi_info[1] = 0x01;													// 用户ID
				wifi_info[2] = 0x02;													// 消息类型
				wifi_info[3] = 4 + n_name + n_password;
				wifi_info[4] = 0x00;													//
				wifi_info[5] = getId(1);										// 消息ID
				wifi_info[6] = getId(2);
				wifi_info[7] = 0x00;													// 状态
				wifi_info[8] = p_encryption->valueint; 	// 加密方式
				wifi_info[9] = n_name;
				wifi_info[10] = n_password;

				for (i = 0; i < n_name; i++)
					wifi_info[11 + i] = p_name->valuestring[i];

				for (i = 0; i < n_password; i++)
					wifi_info[11 + n_name + i] = p_password->valuestring[i];

				wifi_info[11 + n_name + n_password] = calcCheckCode(wifi_info, 11 + n_name + n_password);

				printf("[configureWifi] wifi_info:\n");
				for (i = 0; i < 11 + n_name + n_password + 1; i++)
					printf("%X ", wifi_info[i]);
				printf("\n");

				// 发送wifi配置
				uartSend(uart_hd, wifi_info, 11 + n_name + n_password + 1);
			}
			cJSON_Delete(p_json);
		}
	}
	else
	{
		printf("param_audio_run文件不存在,请联系作者\n");
	}
	fclose(fp);
}

/*******************************************************
* 函数名:			getWifiStatus
* 函数说明:		获取wifi连接状态
* 参数:
* 返回值:			无
*******************************************************/
void getWifiStatus()
{
	int i = 0;
	// 发送主控消息获取wifi_status
	/*
	{
		"type": "status",
		"content": {
		"query": "wifi" // 查询 AIUI WIFI 状态信息
		}
	}
	*/
	char* c_wifi_json;
	cJSON* p_wifi_json1, * p_wifi_json2;
	p_wifi_json1 = cJSON_CreateObject(); // 创建项目
	cJSON_AddItemToObject(p_wifi_json1, "type", cJSON_CreateString("status"));
	cJSON_AddItemToObject(p_wifi_json1, "content", p_wifi_json2 = cJSON_CreateObject());	// 在项目上添加项目
	cJSON_AddStringToObject(p_wifi_json2, "query", "wifi");	// 在项目上的项目上添加字符串，这说明cJSON是可以嵌套的

	c_wifi_json = cJSON_Print(p_wifi_json1);
	//printf("c_wifi_json:\n%s\n", c_wifi_json);	//此时out指向的字符串就是JSON格式的了

	unsigned char c_wifi[7 + strlen(c_wifi_json) + 1];
	memset(c_wifi, 0, 7 + strlen(c_wifi_json) + 1);
	c_wifi[0] = 0xA5;
	c_wifi[1] = 0x01;
	c_wifi[2] = 0x05;
	//sprintf(&c_wifi[3], "%02X", (unsigned char)strlen(c_wifi_json));
	c_wifi[3] = strlen(c_wifi_json);
	c_wifi[4] = 0x00;
	c_wifi[5] = getId(1);
	c_wifi[6] = getId(2);
	for (i = 0; i < strlen(c_wifi_json); i++)
		c_wifi[7 + i] = c_wifi_json[i];

	// 计算校检码
	c_wifi[7 +strlen(c_wifi_json)] = calcCheckCode(c_wifi, 7 +strlen(c_wifi_json));

	printf("c_wifi:\n");
	for (i = 0; i < 7 + strlen(c_wifi_json) + 1; i++)
		printf("%02X ", c_wifi[i]);
	printf("\n");

	// 发送wifi配置
	uartSend(uart_hd, c_wifi, 7 + strlen(c_wifi_json) + 1);

	cJSON_Delete(p_wifi_json1);
}

/*******************************************************
* 函数名:			sendAckMsg
* 函数说明:		发送消息
* 参数:
*	type 				消息类型 1 - 确认消息 2 - 握手消息
*	low_id 		低字节消息ID
*	high_id 		高字节消息ID
* 返回值:		无
*******************************************************/
void sendMsg(int type, unsigned char low_id, unsigned char high_id)
{
	// 拼接确认消息
	ack_buf[0] = 0xA5;
	ack_buf[1] = 0x01;
	if (type == 1)
		ack_buf[2] = 0xff; // 0xff是确认消息类型
	else if (type == 2)
		ack_buf[2] = 0x01;
	ack_buf[3] = 0x04;
	ack_buf[4] = 0x00;
	ack_buf[5] = low_id;	 // 消息ID同要收到消息ID相同
	ack_buf[6] = high_id; 
	ack_buf[7] = 0xA5;
	ack_buf[8] = 0x00;
	ack_buf[9] = 0x00;
	ack_buf[10] = 0x00;

	// 计算校检码
	ack_buf[11] = calcCheckCode(ack_buf, 11);

	// 发送确认消息
	uartSend(uart_hd, ack_buf, 12);
}

/*******************************************************
* 函数名:			gzUncompress
* 函数说明:		解压gzip压缩后的数据
* 参数:
*	zdata 		待解压的gzip数据
*	nzdata	待解压的gzip数据的长度
*	data 			解压后存放数据的buf
*	ndata 		解压后存放数据buf的长度
* 返回值:	无
*******************************************************/
int gzUncompress(Byte* zdata, uLong nzdata, Byte* data, uLong* ndata)
{
	int err = 0;
	z_stream d_stream = { 0 }; 
	static char dummy_head[2] = { 0x8 + 0x7 * 0x10, (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF, };
	d_stream.zalloc = NULL;
	d_stream.zfree = NULL;
	d_stream.opaque = NULL;
	d_stream.next_in = zdata;
	d_stream.avail_in = 0;
	d_stream.next_out = data;
	// 只有设置为MAX_WBITS + 16才能在解压带header和trailer的文本
	if(inflateInit2(&d_stream, MAX_WBITS + 16) != Z_OK) 
		return -1;
	//if(inflateInit2(&d_stream, 47) != Z_OK) return -1;
	while(d_stream.total_out < *ndata && d_stream.total_in < nzdata) 
	{
		d_stream.avail_in = d_stream.avail_out = 1; 
		if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) 
			break;
		if(err != Z_OK) 
		{
			if(err == Z_DATA_ERROR) 
			{
				d_stream.next_in = (Bytef*)dummy_head;
				d_stream.avail_in = sizeof(dummy_head);
				if((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK) 
				{
					return -1;
				}
			} 
			else 
				return -1;
		}
	}
	if(inflateEnd(&d_stream) != Z_OK) 
		return -1;
	*ndata = d_stream.total_out;
	return 0;
}

/*******************************************************
* 函数名:			getTimeStatus
* 函数说明:		打印当前时间
* 参数:
* 返回值:			
*******************************************************/
void getTimeStatus()
{
    time_t nowTime;
    struct tm* timeInfo;
    time(&nowTime);
    timeInfo = localtime(&nowTime);
    int nhour, nmin, nsec;
    nhour = timeInfo->tm_hour;
    nmin = timeInfo->tm_min;
    nsec = timeInfo->tm_sec;
    printf("time:%d:%d:%d\n", nhour, nmin, nsec);
}

/*******************************************************
* 函数名:			threadTTS
* 函数说明:		语音合成线程函数
* 参数:
* 返回值:			NULL
*******************************************************/
void* threadTTS(void*)
{	
	// 在接收到语音识别结果之后 以线程的方式调用语音合成,可以使用传入参数的方式将需要进行合成的字符串传入函数内
	tts_finished = 0;
	// 串口合成
	std::string str = "测试语音合成";
	textToSpeech(str.c_str());

	while (1)
	{
		usleep(1000);
		if (tts_finished == 1)
			break;
		printf("while tts_finished:%d\n", tts_finished);
	}
	return NULL;
}

/*******************************************************
* 函数名:			processRecv
* 函数说明:		分析接收到的串口消息
* 参数:
*	buf 				接收到的完整串口消息数据
*	len 				接收到的完整串口消息数据长度
* 返回值:		无
*******************************************************/
void processRecv(unsigned char* buf, int len)
{
	int i = 0, index = 0;
	// 判断校验位是否正确
	unsigned char check = 0;
	for (index = 0; index < len - 1; index++)
		check += buf[index];
	check = ~check + 1;

	if (check == buf[len - 1])
	{
		//try catch异常情况
		// 判断msg类型
		if (buf[2] == 0xff)
		{
			// 对确认消息先不处理
			printf("buf[2] = 0xff\n");
			//return;
		}

		printf("buf:\n");
		for (i = 0; i < len; i++)
			printf("%X ", buf[i]);
		printf("\n");

		// 发送确认消息
		sendMsg(1, buf[5], buf[6]);

		// 只有第三位数据为0x04时进行处理
		if (buf[2] == 0x04)
		{
			// 对0x04消息进行gzip解压
			uLong u_destbuf_len = len * 100; // 此处长度需要足够大以容纳解压缩后数据
			char* u_destbuf = (char*)calloc((uInt)u_destbuf_len, 1);

			gzUncompress((Byte*)buf + 7, (uLong)len - 1, (Byte*)u_destbuf, (uLong*)&u_destbuf_len);
			printf("JSON:\n%s\n", (char*)u_destbuf);
			
			//解析json数据
			/*
				"content"
					"info"
						"params"
							"data"->child
								"sub"
				判断sub字段的值是否位nlp
				如果是
				"content"
					"result"
						"intent"
							"text"
				从text字段中获取语义分析结果										
			*/
									
			cJSON* p_json = cJSON_Parse((char*)u_destbuf);
			if (NULL != p_json)
			{
				printf("parse json successful\n");
				cJSON* p_type = cJSON_GetObjectItem(p_json, "type");
				// aiui_event处理
				if ((NULL != p_type) && (0 == strcmp(p_type->valuestring, "aiui_event")))
				{
					printf("type:%s\n", p_type->valuestring);
					cJSON* p_content = cJSON_GetObjectItem(p_json, "content");
					if (NULL != p_content)
					{
						cJSON* p_arg1 = cJSON_GetObjectItem(p_content, "arg1");
						cJSON* p_arg2 = cJSON_GetObjectItem(p_content, "arg2");
						cJSON* p_event_type = cJSON_GetObjectItem(p_content, "eventType");
						if ((NULL != p_event_type) && (NULL != p_arg1) && (NULL != p_arg2))
						{
							printf("eventType:%d\n", p_event_type->valueint);
							// EVENT_RESULT
							if (1 == p_event_type->valueint)
							{	
								// data字段为result数据
	                            // info字段为描述数据的json
	                            printf("EVENT_RESULT\n");
	                            cJSON* p_info = cJSON_GetObjectItem(p_content, "info");
	                            if (NULL != p_info)
	                            {
	                            	cJSON* p_sub = cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(p_info, "data")->child, "params"), "sub");
		                            if ((NULL != p_sub) && 0 == (strcmp(p_sub->valuestring, "nlp")))
		                            {    
		                                cJSON* p_text = cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(p_content, "result"), "intent"), "text");
		                                if (NULL != p_text)
		                                {    
	                                        if (0 != strcmp(p_text->valuestring, ""))
	                                        {    
	                                            // 获取到的语音识别结果 
	                                            printf("result:%s\n", p_text->valuestring);
	                                        }       
		                                }
		                            }
	                            }
							}
							// EVENT_ERROR
							else if (2 == p_event_type->valueint)
							{
								cJSON* p_info = cJSON_GetObjectItem(p_content, "info");
								// arg1字段为错误码
								// info字段为错误描述信息
								printf("EVENT_ERROR\n");
								if (10106 == p_arg1->valueint)
								{
									printf("MSP_ERROR_INVALID_PARA 10106 参数名称错误\n");
								}
								else if (10107 == p_arg1->valueint)
								{
									printf("MSP_ERROR_INVALID_PARA_VALUE 10107 参数取值错误\n");
								}
								else if (10116 == p_arg1->valueint)
								{
									printf("MSP_ERROR_NOT_FOUND 10116 云端无对应的scene场景参数\n");
								}
								else if (10120 == p_arg1->valueint)
								{
									printf("MSP_ERROR_NO_RESPONSE_DATA 10120 结果等待超时\n");

									char path[256] = { 0 };
									sprintf(path, "%s/src/audio/Audio/src/param_audio_run", getenv("CHARLENE_HOME"));
									configureWifi(path);
									
									getWifiStatus();
								}
								else if (16005 == p_arg1->valueint)
								{
									printf("MSP_ERROR_LMOD_RUNTIME_EXCEPTION 16005 MSC 内部错误\n");
								}
								else if (20001 == p_arg1->valueint)
								{
									printf("ERROR_NO_NETWORK 20001 无有效的网络连接\n");

									char path[256] = { 0 };
									sprintf(path, "%s/src/audio/Audio/src/param_audio_run", getenv("CHARLENE_HOME"));
									configureWifi(path);
									
									getWifiStatus();
								}
								else if (20002 == p_info->valueint)
								{
									printf("ERROR_NETWORK_TIMEOUT 20002 网络连接超时\n");
								}
								else if (20003 == p_info->valueint)
								{
									printf("ERROR_NET_EXPECTION 20003 网络连接发生异常\n");
								}
								else if (20004 == p_info->valueint)
								{
									printf("ERROR_INVALID_RESULT 20004 无有效的结果\n");	
								}
								else if (20005 == p_info->valueint)
								{
									printf("ERROR_NO_MATCH 20005 无匹配结果\n");
								}
								else if (20006 == p_info->valueint)
								{
									printf("ERROR_AUDIO_RECORD 20006 录音失败\n");
								}
								else if (21001 == p_info->valueint)
								{
									printf("ERROR_COMPONENT_NOT_INSTALLED 21001 没有安装服务组件\n");
								}
								else if (21020 == p_info->valueint)
								{
									printf("ERROR_SERVICE_BINDER_DIED 21020 与服务的绑定已消亡\n");
								}
								else if (22001 == p_info->valueint)
								{
									printf("ERROR_LOCAL_NO_INIT 22001 本地引擎未初始化\n");
								}
								else if (20999 == p_info->valueint)
								{
									printf("ERROR_UNKNOWN 20999 未知错误\n");
								}
							}
							// EVENT_STATE
							else if (3 == p_event_type->valueint)
							{
								// arg1字段
								// STATE_IDLE(空闲状态)
								// STATE_READY(就绪状态)
								// STATE_WORKING(工作状态)
								printf("EVENT_STATE\n");
								printf("arg1:%d\n", p_arg1->valueint);
							}
							// EVENT_WAKEUP
							else if (4 == p_event_type->valueint)
							{
								//info字段为唤醒结果json
								//"power":12342435436	唤醒能量值
								//"beam":3				拾音波束号,唤醒成功后阵列将在该波束方向上拾音
								//"angle":180			唤醒角度
								//"channel":5			唤醒声道,即麦克风编号,表示该声道的音频质量最好
								//"CMScore":132			声道对应的唤醒得分
								printf("EVENT_WAKEUP\n");

								//关闭播报
								closeVoice();

								tts_finished = 1;

								cJSON* p_info = cJSON_GetObjectItem(p_content, "info");
							/*
								LOG(INFO) << "------------------------------------" << std::endl;
								LOG(INFO) << "[EVENT_WAKEUP] power:" << cJSON_GetObjectItem(p_info, "power")->valueint << std::endl;
								LOG(INFO) << "[EVENT_WAKEUP] beam:" << cJSON_GetObjectItem(p_info, "beam")->valueint << std::endl;
								LOG(INFO) << "[EVENT_WAKEUP] angle:" << cJSON_GetObjectItem(p_info, "angle")->valueint << std::endl;
								LOG(INFO) << "[EVENT_WAKEUP] channel:" << cJSON_GetObjectItem(p_info, "channel")->valueint << std::endl;
								LOG(INFO) << "[EVENT_WAKEUP] CMScore:" << cJSON_GetObjectItem(p_info, "CMScore")->valueint << std::endl;
								LOG(INFO) << "[EVENT_WAKEUP] tts_finished:" << tts_finished << std::endl;
								LOG(INFO) << "------------------------------------" << std::endl;
							*/

								// 设置拾音波束
								char* c_set_beam_json;
								cJSON* p_set_beam_json1, * p_set_beam_json2;
								p_set_beam_json1 = cJSON_CreateObject();	//创建项目
								cJSON_AddItemToObject(p_set_beam_json1, "type", cJSON_CreateString("aiui_msg"));
								cJSON_AddItemToObject(p_set_beam_json1, "content", p_set_beam_json2 = cJSON_CreateObject());	//在项目上添加项目
								// 在项目上的项目上添加字符串，这说明cJSON是可以嵌套的
								cJSON_AddNumberToObject(p_set_beam_json2, "msg_type", 9);	//设置拾音波束号
								cJSON_AddNumberToObject(p_set_beam_json2, "arg1", 0);		//拾音波束号
								cJSON_AddNumberToObject(p_set_beam_json2, "arg2", 0);
								cJSON_AddStringToObject(p_set_beam_json2, "params", "");

								c_set_beam_json = cJSON_Print(p_set_beam_json1);

								unsigned char c_set_beam[7 + strlen(c_set_beam_json) + 1];
								memset(c_set_beam, 0, 7 + strlen(c_set_beam_json) + 1);
								c_set_beam[0] = 0xA5;
								c_set_beam[1] = 0x01;
								c_set_beam[2] = 0x05;
								c_set_beam[3] = strlen(c_set_beam_json);
								c_set_beam[4] = 0x00;
								c_set_beam[5] = getId(1);
								c_set_beam[6] = getId(2);
								for (i = 0; i < strlen(c_set_beam_json); i++)
									c_set_beam[7 + i] = c_set_beam_json[i];

								c_set_beam[7 + strlen(c_set_beam_json)] = calcCheckCode(c_set_beam, 7 + strlen(c_set_beam_json));

								uartSend(uart_hd, c_set_beam, 7 + strlen(c_set_beam_json) + 1);	

								cJSON_Delete(p_set_beam_json1);
							}
							// EVENT_SLEEP
							else if (5 == p_event_type->valueint)
							{
								printf("EVENT_SLEEP\n");
								
								// 手动唤醒AIUI
								char* c_wake_up_json;
								cJSON* p_wake_up_json1, * p_wake_up_json2;
								p_wake_up_json1 = cJSON_CreateObject();	// 创建项目
								cJSON_AddItemToObject(p_wake_up_json1, "type", cJSON_CreateString("aiui_msg"));
								cJSON_AddItemToObject(p_wake_up_json1, "content", p_wake_up_json2 = cJSON_CreateObject());	// 在项目上添加项目
								// 在项目上的项目上添加字符串，这说明cJSON是可以嵌套的
								cJSON_AddNumberToObject(p_wake_up_json2, "msg_type", 7);		// 手动唤醒AIUI
								cJSON_AddNumberToObject(p_wake_up_json2, "arg1", 0);				// 设置拾音波束号
								cJSON_AddNumberToObject(p_wake_up_json2, "arg2", 0);
								cJSON_AddStringToObject(p_wake_up_json2, "params", "");

								c_wake_up_json = cJSON_Print(p_wake_up_json1);
								//printf("c_wake_up_json:\n%s\n", c_wake_up_json);	// 此时out指向的字符串就是JSON格式的了

								unsigned char c_wake_up[7 + strlen(c_wake_up_json) + 1];
								memset(c_wake_up, 0, 7 + strlen(c_wake_up_json) + 1);
								c_wake_up[0] = 0xA5;
								c_wake_up[1] = 0x01;
								c_wake_up[2] = 0x05;
								//sprintf(&c_wake_up[3], "%02X", (unsigned char)strlen(c_wake_up_json));
								c_wake_up[3] = strlen(c_wake_up_json);
								c_wake_up[4] = 0x00;
								c_wake_up[5] = getId(1);
								c_wake_up[6] = getId(2);
								for (i = 0; i < strlen(c_wake_up_json); i++)
									c_wake_up[7 + i] = c_wake_up_json[i];

								c_wake_up[7 + strlen(c_wake_up_json)] = calcCheckCode(c_wake_up, 7 + strlen(c_wake_up_json));

								uartSend(uart_hd, c_wake_up, 7 + strlen(c_wake_up_json) + 1);	

								cJSON_Delete(p_wake_up_json1);
							}
							// EVENT_VAD
							else if (6 == p_event_type->valueint)
							{
								// arg1标识前后端点或者音量信息
								// 0(前端点) 1(音量) 2(后端点)
								// 当arg1=1时,arg2为音量大小	
								printf("EVENT_VAD\n");
								if (2 == p_arg1->valueint)
								{
									printf("arg1:%d arg2:%d\n", p_arg1->valueint, p_arg2->valueint);
								}
							}
						}
					}
				}
				// wifi_status处理
				else if ((NULL != p_type) && (0 == strcmp(p_type->valuestring, "wifi_status")))
				{
					printf("wifi_status处理\n");
				}
				else if ((NULL != p_type) && (0 == strcmp(p_type->valuestring, "tts_event")))
				{
					printf("tts_event处理\n");
					cJSON* p_content = cJSON_GetObjectItem(p_json, "content");
					if (NULL != p_content)
					{
						cJSON* p_event_type = cJSON_GetObjectItem(p_content, "eventType");
						printf("p_event_type->valueint:%d\n", p_event_type->valueint);
						if ((NULL != p_event_type) && (p_event_type->valueint == 1))
						{	
							cJSON* p_error = cJSON_GetObjectItem(p_content, "error");
							if (NULL != p_error)
								printf("tts finished, error:%d\n", p_error->valueint);
							tts_finished = 1;

						}
						else if ((NULL != p_event_type) && (p_event_type->valueint == 0))
						{
							printf("tts ing\n");
							getTimeStatus();
							tts_finished = 0;
						}
					}
				}
			}
			cJSON_Delete(p_json);
			free(u_destbuf);
		}
	}
}

/*******************************************************
* 函数名:			uartRec
* 函数说明:		接收串口消息函数
* 参数:
*	msg 					接收到的串口消息数据
*	msglen 			接收到的串口消息数据长度
*	user_data		用户数据(附属参数)
* 返回值:			无
*******************************************************/
void uartRec(const void *msg, unsigned int msglen, void *user_data)
{
	//printf("uart recv %d\n", msglen);

	// 过滤不以A5 01开头的无效数据
	if(big_buf == NULL && recv_index + msglen >= 2)
	{
		if(recv_index == 0)
		{
			if(((unsigned char*)msg)[0] != SYNC_HEAD | ((unsigned char*)msg)[1] != SYNC_HEAD_SECOND)
			{
				printf("recv data not SYNC HEAD, drop\n");
				return;
			}
		}
		else if(recv_index == 1)
		{
			if(recv_buf[0] != SYNC_HEAD | ((unsigned char*)msg)[0] != SYNC_HEAD_SECOND)
			{
				printf("recv data not SYNC HEAD, drop\n");
				recv_index = 0;
				return;
			}
		}
	}

	// 不断接收串口字节，构造完成消息
	int copy_len;
	if(big_buf != NULL)
	{
		copy_len = big_buf_len - big_buf_index < msglen? big_buf_len - big_buf_index : msglen;
		memcpy(big_buf + big_buf_index, msg, copy_len);
		big_buf_index += copy_len;
		if(big_buf_index < big_buf_len) 
			return;
	}
	else 
	{
		copy_len = RECV_BUF_LEN - recv_index < msglen? RECV_BUF_LEN - recv_index : msglen;
		memcpy(recv_buf + recv_index, msg, copy_len);
		if((recv_index + copy_len) > PACKET_LEN_BIT) 
		{
			int content_len = recv_buf[PACKET_LEN_BIT] << 8 | recv_buf[PACKET_LEN_BIT - 1];
			if(content_len != MSG_NORMAL_LEN)
			{
				big_buf_index = 0;
				big_buf_len = content_len + MSG_EXTRA_LEN;
				big_buf = malloc(big_buf_len);
				//printf("uart malloc buf %x len %d\n", (unsigned int)big_buf, big_buf_len);
				memset(big_buf, big_buf_len, 0);
				memcpy(big_buf, recv_buf, recv_index);
				big_buf_index += recv_index;
				recv_index = 0;
				
				return uartRec(msg, msglen, user_data);
			}
		}
		recv_index += copy_len;
		if(recv_index < RECV_BUF_LEN) 
			return;
	}

	// 已接收完一条完整消息
	if(big_buf != NULL)
	{
		processRecv((unsigned char*)big_buf, big_buf_len);   //接受消息处理
		big_buf_len = 0;
		big_buf_index = 0;
		//printf("uart free buf %x\n", (unsigned int)big_buf);
		free(big_buf);
		big_buf = NULL;
	}
	else
	{
		processRecv(recv_buf, RECV_BUF_LEN);  // 接受消息处理
		recv_index = 0;
	}

	// 读取的数据中包含下一条消息的开头部分
	if(copy_len < msglen)
	{
		printf("multi msg in one stream left %d byte\n", msglen - copy_len);
		uartRec(msg + copy_len, msglen - copy_len, user_data);
	}
}

#define UART_DEVICE "/dev/xunfei"

int main(int argc, char* argv[])
{
	// 程序退出控制变量
	g_quit = 0;
	// ctrl-c信号处理
	signal(SIGINT, sigHandler);
	// 初始化串口合成状态
	tts_finished = 1;

	int ret = 0;
	printf("Audio run\n");
	// 初始化uart串口
	ret = uartInit(&uart_hd, UART_DEVICE, 115200, uartRec, NULL);
	if (0 != ret)
	{
		printf("uartInit error ret = %d\n", ret);
		printf("please check up USB port or USB power supply\n");
		return -1;
	}

    // 发起主动握手
    sendMsg(2, getId(1), getId(2));

    // 关闭AIUI自身播报
	closeVoice();

    // 串口合成,旧版本因为第一次合成会失败,于是发送一个空合成避开合成失败
    textToSpeech("");

	// 线程ID
	pthread_t tid;
    // 创建线程，接收数据
	ret = pthread_create(&tid, NULL, threadTTS, NULL);
	if(0 != ret)
	{
		printf("threadTTS thread create failed!\n");
	}
	pthread_detach(tid);

	while(g_quit == 0)
	{
		sleep(3);
	}

	// 关闭uart串口
	uartUninit(uart_hd);

	// 接收到ctrl + c的关闭信号
	printf("ctrl c signal\n");

    return 0;
}