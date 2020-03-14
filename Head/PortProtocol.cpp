#include "./PortProtocol.h"
int openPort()
{
	_serialFd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (_serialFd == -1)
	{
		cout << "Open serial port failed." << endl;
		return _errorCode = -1;
	}

	termios tOption;                                // 串口配置结构体
	tcgetattr(_serialFd, &tOption);                 //获取当前设置
	cfmakeraw(&tOption);
	cfsetispeed(&tOption, B9600);                 // 接收波特率
	cfsetospeed(&tOption, B9600);                 // 发送波特率
	tcsetattr(_serialFd, TCSANOW, &tOption);
	tOption.c_cflag &= ~PARENB;
	tOption.c_cflag &= ~CSTOPB;
	tOption.c_cflag &= ~CSIZE;
	tOption.c_cflag |= CS8;
	tOption.c_cflag &= ~INPCK;
	tOption.c_cflag |= (B9600 | CLOCAL | CREAD);  // 设置波特率，本地连接，接收使能
	tOption.c_cflag &= ~(INLCR | ICRNL);
	tOption.c_cflag &= ~(IXON);
	tOption.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	tOption.c_oflag &= ~OPOST;
	tOption.c_oflag &= ~(ONLCR | OCRNL);
	tOption.c_iflag &= ~(ICRNL | INLCR);
	tOption.c_iflag &= ~(IXON | IXOFF | IXANY);
	tOption.c_cc[VTIME] = 1;                        //只有设置为阻塞时这两个参数才有效
	tOption.c_cc[VMIN] = 1;
	tcflush(_serialFd, TCIOFLUSH);                   //TCIOFLUSH刷新输入、输出队列。

	cout << "Serial preparation complete." << endl;
	return _errorCode = 0;
}
/*
*	@Brief: close port
*/
int closePort()
{
	tcflush(_serialFd, TCIOFLUSH);
	if (-1 == close(_serialFd))
	{
		_errorCode = -1;
		cout << "Serial closing failed." << endl;
	}
	else
	{
		_errorCode = 0;
		cout << "Serial closing sucessfully." << endl;
	}
	return _errorCode;
}
/*
*	@Brief: send control data
*/
int send(control_frame& ctl_f)
{
	ctl_f.sof = FSOF;
	ctl_f.eof = FEOF;
	tcflush(_serialFd, TCOFLUSH);

	int sendCount;
	try
	{
		sendCount = write(_serialFd, &ctl_f, sizeof(ctl_f));
		_errorCode = 0;
	}
	catch (exception e)
	{
		cout << e.what() << endl;
		return _errorCode = -1;
	}

	return _errorCode;
}
/*
*	@Brief:receive feedback data
*/
int receive(feedback_frame& fdk_f)
{
	fdk_f.sof = FSOF;
	fdk_f.eof = FEOF;
	memset(&fdk_f, 0, sizeof(feedback_frame));

	int readCount = 0;
	while (readCount < sizeof(feedback_frame))
	{

		int onceReadCount;
		try
		{
			onceReadCount = read(_serialFd, ((unsigned char*)&fdk_f) + readCount, sizeof(feedback_frame) - readCount);
		}
		catch (exception e)
		{
			cout << e.what() << endl;
			return _errorCode = -1;
		}

		if (onceReadCount == -1)
		{
			if (errno == EAGAIN)
			{
				continue;
			}


			return _errorCode = -2;
		}

		readCount += onceReadCount;
	}

	tcflush(_serialFd, TCIFLUSH);


}
