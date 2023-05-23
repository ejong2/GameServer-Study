#pragma once

/*----------------
     NetAddress
-----------------*/

class NetAddress
{
public:
    // 기본 생성자 및 매개변수가 있는 생성자 정의
    NetAddress() = default;
    NetAddress(SOCKADDR_IN sockAddr);
    NetAddress(wstring ip, UINT port);

    // 멤버 변수에 접근하기 위한 getter 메서드 정의
    SOCKADDR_IN& GetSockAddr() { return _sockAddr; }
    wstring      GetIpAddress();
    uint16       GetPort() { return ::ntohs(_sockAddr.sin_port); }

public:
    // IP 주소 문자열을 IN_ADDR 구조체로 변환하는 정적 메서드 정의
    static IN_ADDR ip2Address(const WCHAR* ip);

private:
    // 멤버 변수 정의
    SOCKADDR_IN _sockAddr = {};
};
