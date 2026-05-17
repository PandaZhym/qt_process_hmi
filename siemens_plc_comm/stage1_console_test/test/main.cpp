#include <iostream>
#include "snap7.h"

int main()
{
    system("chcp 65001 > nul");  // 控制台切换到 UTF-8，正确显示中文
    TS7Client client;

    // 改成你 PLC 的实际 IP 地址
    const char* plcIp = "192.168.0.1";
    int result = client.ConnectTo(plcIp, 0, 1);

    if (result == 0) {
        std::cout << "连接成功！" << std::endl;
        client.Disconnect();
    } else {
        std::cout << "连接失败，错误码: " << result << std::endl;
        std::cout << "错误说明: " << CliErrorText(result).c_str() << std::endl;
    }

    system("pause");
    return 0;
}
