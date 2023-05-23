#pragma once

/*----------------
     NetAddress
-----------------*/

class NetAddress
{
public:
    // �⺻ ������ �� �Ű������� �ִ� ������ ����
    NetAddress() = default;
    NetAddress(SOCKADDR_IN sockAddr);
    NetAddress(wstring ip, UINT port);

    // ��� ������ �����ϱ� ���� getter �޼��� ����
    SOCKADDR_IN& GetSockAddr() { return _sockAddr; }
    wstring      GetIpAddress();
    uint16       GetPort() { return ::ntohs(_sockAddr.sin_port); }

public:
    // IP �ּ� ���ڿ��� IN_ADDR ����ü�� ��ȯ�ϴ� ���� �޼��� ����
    static IN_ADDR ip2Address(const WCHAR* ip);

private:
    // ��� ���� ����
    SOCKADDR_IN _sockAddr = {};
};
