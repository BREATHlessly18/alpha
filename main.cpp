#include "proccessModule.hpp"
#include <string>
#include <signal.h>

auto sigOperate(int sig) -> void
{
    proccessModule::getInstance().stop();
}

// 主函数须携带采集时间参数，参数为0表示一直采集，其余正整数表示采集时长
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        return -1;
    }

    //signal(SIGINT, sigOperate);

    proccessModule::getInstance().start();

    if (0 == int(atoi(argv[1])))
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    std::this_thread::sleep_for(std::chrono::seconds(atoi(argv[1])));
    proccessModule::getInstance().stop();

    return 0;
}
