#include "proccessModule.hpp"
#include <string>
#include <signal.h>

auto sigOperate(int sig) -> void
{
    proccessModule::getInstance().stop();
}

// ��������Я���ɼ�ʱ�����������Ϊ0��ʾһֱ�ɼ���������������ʾ�ɼ�ʱ��
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
