// test_sig.cpp
#include <signal.h>

#include <execinfo.h>   // backtrace, backtrace_symbols_fd
#include <signal.h>
#include <unistd.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

void crashHandler(int sig) {
    // ① 恢复信号的默认处理，避免递归触发本处理器
    signal(sig, SIG_DFL);

    // ② 打印简单的错误信息（write 是异步信号安全的）
    const char* msg = "\n================================================================\n"
                      "⚠️  Server received fatal signal: ";
    write(STDERR_FILENO, msg, strlen(msg));
    
    // 打印信号编号（简单示意，也可用更详细的信号名）
    char sigBuf[16];
    int len = snprintf(sigBuf, sizeof(sigBuf), "%d\n", sig);
    write(STDERR_FILENO, sigBuf, len);

    const char* msg2 = "Stack trace:\n";
    write(STDERR_FILENO, msg2, strlen(msg2));

    // ③ 获取并打印函数调用栈（backtrace_symbols_fd 是信号安全的）
    void* buffer[128];
    int nptrs = backtrace(buffer, sizeof(buffer) / sizeof(buffer[0]));
    backtrace_symbols_fd(buffer, nptrs, STDERR_FILENO);

    const char* end = "================================================================\n\n";
    write(STDERR_FILENO, end, strlen(end));

    // ④ 重新发送信号，让内核生成 core dump 或执行默认终止
    raise(sig);
}
// ... 把 crashHandler 复制过来 ...
int main() {
    int log_fd = open("./log.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd < 0) exit(1);

    dup2(log_fd, STDOUT_FILENO);
    dup2(log_fd, STDERR_FILENO);
    close(log_fd);  // 关闭多余的文件描述符

    // 同样，设置无缓冲
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

    // 注册信号处理器（同 main）
    struct sigaction sa;
    sa.sa_handler = crashHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;
    sigaction(SIGSEGV, &sa, nullptr);

    int* p = nullptr;
    *p = 42; // 故意崩溃
    return 0;
}