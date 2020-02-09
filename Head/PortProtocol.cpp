#include "./PortProtocol.h"
int openPort()
{
	_serialFd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (_serialFd == -1)
	{
		cout << "Open serial port failed." << endl;
		return _errorCode = -1;
	}

	termios tOption;                                // �������ýṹ��
	tcgetattr(_serialFd, &tOption);                 //��ȡ��ǰ����
	cfmakeraw(&tOption);
	cfsetispeed(&tOption, B9600);                 // ���ղ�����
	cfsetospeed(&tOption, B9600);                 // ���Ͳ�����
	tcsetattr(_serialFd, TCSANOW, &tOption);
	tOption.c_cflag &= ~PARENB;
	tOption.c_cflag &= ~CSTOPB;
	tOption.c_cflag &= ~CSIZE;
	tOption.c_cflag |= CS8;
	tOption.c_cflag &= ~INPCK;
	tOption.c_cflag |= (B9600 | CLOCAL | CREAD);  // ���ò����ʣ��������ӣ�����ʹ��
	tOption.c_cflag &= ~(INLCR | ICRNL);
	tOption.c_cflag &= ~(IXON);
	tOption.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	tOption.c_oflag &= ~OPOST;
	tOption.c_oflag &= ~(ONLCR | OCRNL);
	tOption.c_iflag &= ~(ICRNL | INLCR);
	tOption.c_iflag &= ~(IXON | IXOFF | IXANY);
	tOption.c_cc[VTIME] = 1;                        //ֻ������Ϊ����ʱ��������������Ч
	tOption.c_cc[VMIN] = 1;
	tcflush(_serialFd, TCIOFLUSH);                  //TCIOFLUSHˢ�����롢������С�

	cout << "Serial preparation complete." << endl;
	return _errorCode = 0;
}

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