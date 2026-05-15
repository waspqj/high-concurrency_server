#include "Http.hpp"

std::string generateRainbowPage(const std::string& name) {
     // 颜色列表（可根据喜好调整）
    std::vector<std::string> colors = {
        "#FF6B6B", "#FF8E72", "#FFA07A", "#FFD93D", 
        "#6BCB77", "#4D96FF", "#9B59B6"
    };

    // 为每个字符生成带颜色动画的 <span>
    std::string spans;
    for (size_t i = 0; i < name.size(); ++i) {
        std::string color = colors[i % colors.size()];
        spans += "<span style='color:" + color + "; animation-delay:" 
              + std::to_string(0.05 * i) + "s'>" + name[i] + "</span>";
    }

    // HTML 模板（带毛玻璃卡片和动画）
    std::string html = 
        "<!DOCTYPE html>"
        "<html lang='zh'>"
        "<head><meta charset='UTF-8'><title>Welcome</title></head>"
        "<body style='margin:0; height:100vh; display:flex; align-items:center; justify-content:center;"
        "background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); font-family: system-ui;'>"
        "<div style='background: rgba(255,255,255,0.15); backdrop-filter: blur(10px);"
        "border-radius: 24px; padding: 3rem 4rem; box-shadow: 0 8px 32px rgba(0,0,0,0.2); text-align: center;'>"
        "<h1 style='color: white; font-weight: 400; margin: 0 0 1rem 0;'>"
        "<span style='font-size: 1.5em;'>✨ Welcome, </span>"
        "<div style='font-size: 4rem; font-weight: 700; letter-spacing: 2px; margin-top: 0.5rem;'>"
        + spans +
        "</div>"
        "<p style='color: rgba(255,255,255,0.9); margin-top: 2rem; font-size: 1.2rem;'>很高兴见到你！</p>"
        "<p style='text-align:center; color:rgba(255,255,255,0.8); font-size:1rem; margin-top:1rem;'>准备好了吗?<a href='/Login.html' style='color:#ffd700;'>登录试试吧</a></p>"
        "</h1></div>"
        "<style>"
        "@keyframes jumpIn { 0% { transform: translateY(20px); opacity: 0; } 100% { transform: translateY(0); opacity: 1; } }"
        "h1 span span { display: inline-block; animation: jumpIn 0.3s ease forwards; }"
        "</style>"
        "</body></html>";

    return html;
}
std::string generatePunishPage() {
    std::string html =
        "<!DOCTYPE html>"
        "<html lang=\"zh\">"
        "<head>"
        "<meta charset=\"UTF-8\">"
        "<title>😈 惩罚时间</title>"
        "<style>"
        "* { margin:0; padding:0; box-sizing:border-box; }"
        "body { height:100vh; background: linear-gradient(135deg, #1a1a2e, #16213e, #0f3460); display:flex; align-items:center; justify-content:center; font-family:'Segoe UI',system-ui,sans-serif; color:white; }"
        ".card { background: rgba(255,255,255,0.08); backdrop-filter:blur(20px); border-radius:24px; padding:3rem; box-shadow:0 20px 50px rgba(0,0,0,0.5); text-align:center; max-width:500px; width:90%; }"
        "h1 { font-size:2.5rem; margin-bottom:0.5rem; }"
        ".timer { font-size:4rem; font-weight:bold; color:#ffd700; margin:1rem 0; }"
        "input { width:100%; padding:12px 20px; font-size:1.2rem; border:none; border-radius:12px; outline:none; text-align:center; margin:1rem 0; }"
        "button { background:#ff6b6b; border:none; color:white; font-size:1.2rem; padding:12px 40px; border-radius:50px; font-weight:bold; cursor:pointer; transition:0.3s; margin-top:0.5rem; }"
        "button:hover { transform:scale(1.05); box-shadow:0 0 20px #ff6b6b; }"
        ".hint { font-size:1.2rem; min-height:2rem; margin-top:1rem; }"
        ".pig { font-size:10rem; color:#ff4757; display:none; margin:1rem 0; }"
        ".success { display:none; }"
        ".success a { color:#ffd700; font-size:1.2rem; text-decoration:none; background:rgba(255,255,255,0.15); padding:10px 35px; border-radius:30px; }"
        "</style>"
        "</head>"
        "<body>"
        "<div class=\"card\">"
        "<h1>🐷 猜数字惩罚</h1>"
        "<p style=\"margin-bottom:1rem;\">猜一个 1 ~ 100 的数字，只有 <strong>20 秒</strong>！</p>"
        "<div class=\"timer\" id=\"timer\">20</div>"
        "<input type=\"number\" id=\"guessInput\" placeholder=\"输入你的猜测\" min=\"1\" max=\"100\" autofocus>"
        "<button id=\"submitBtn\">猜！</button>"
        "<div class=\"hint\" id=\"hint\"></div>"
        "<div class=\"pig\" id=\"pig\">🐷<br>猪</div>"
        "<div class=\"success\" id=\"success\">"
        "<p style=\"font-size:1.5rem; margin-bottom:1rem;\">🎉 你猜对了！惩罚解除！</p>"
        "<a href=\"javascript:history.back()\">🔙 返回游戏</a>"
        "</div>"
        "</div>"
        "<script>"
        "let secret = Math.floor(Math.random() * 100) + 1;"
        "let timeLeft = 20;"
        "let timerInterval;"
        "let gameOver = false;"
        ""
        "const timerEl = document.getElementById('timer');"
        "const guessInput = document.getElementById('guessInput');"
        "const submitBtn = document.getElementById('submitBtn');"
        "const hintEl = document.getElementById('hint');"
        "const pigEl = document.getElementById('pig');"
        "const successEl = document.getElementById('success');"
        ""
        "function startTimer() {"
        "  timerInterval = setInterval(() => {"
        "    timeLeft--;"
        "    timerEl.textContent = timeLeft;"
        "    if (timeLeft <= 5) timerEl.style.color = '#ff4757';"
        "    if (timeLeft <= 0) {"
        "      clearInterval(timerInterval);"
        "      timeUp();"
        "    }"
        "  }, 1000);"
        "}"
        ""
        "function timeUp() {"
        "  if (gameOver) return;"
        "  gameOver = true;"
        "  guessInput.disabled = true;"
        "  submitBtn.disabled = true;"
        "  pigEl.style.display = 'block';"
        "  hintEl.textContent = `答案是 ${secret}`;"
        "}"
        ""
        "function guess() {"
        "  if (gameOver) return;"
        "  let val = parseInt(guessInput.value);"
        "  if (isNaN(val) || val < 1 || val > 100) {"
        "    hintEl.textContent = '请输入 1~100 之间的数字';"
        "    return;"
        "  }"
        "  if (val === secret) {"
        "    clearInterval(timerInterval);"
        "    gameOver = true;"
        "    guessInput.style.display = 'none';"
        "    submitBtn.style.display = 'none';"
        "    timerEl.style.display = 'none';"
        "    successEl.style.display = 'block';"
        "  } else if (val < secret) {"
        "    hintEl.textContent = '太小了，再大一点';"
        "  } else {"
        "    hintEl.textContent = '太大了，再小一点';"
        "  }"
        "  guessInput.value = '';"
        "  guessInput.focus();"
        "}"
        ""
        "submitBtn.addEventListener('click', guess);"
        "guessInput.addEventListener('keypress', (e) => { if (e.key === 'Enter') guess(); });"
        "startTimer();"
        "</script>"
        "</body>"
        "</html>";
    return html;
}
std::string generateGamePage(const std::string& username) {
    std::string html = 
        "<!DOCTYPE html>"
        "<html lang=\"zh\">"
        "<head>"
        "<meta charset=\"UTF-8\">"
        "<title>🎲 骰子对战</title>"
        "<style>"
        "* { margin:0; padding:0; box-sizing:border-box; }"
        "body { height:100vh; background: linear-gradient(135deg, #0f0c29, #302b63, #24243e); display:flex; align-items:center; justify-content:center; font-family:'Segoe UI',system-ui,sans-serif; color:white; flex-direction:column; }"
        ".container { background: rgba(255,255,255,0.08); backdrop-filter:blur(20px); border-radius:30px; padding:3rem; box-shadow:0 20px 50px rgba(0,0,0,0.5); text-align:center; }"
        "h1 { font-size:2.5rem; margin-bottom:10px; }"
        ".player-name { font-size:1.5rem; color:#ffd700; margin-bottom:2rem; }"
        ".dice-container { display:flex; justify-content:center; gap:2rem; margin:2rem 0; }"
        ".die { width:90px; height:90px; background:white; border-radius:20px; display:flex; align-items:center; justify-content:center; font-size:3.5rem; font-weight:bold; color:#333; box-shadow:0 10px 20px rgba(0,0,0,0.3), inset 0 -4px 0 rgba(0,0,0,0.15); transition:transform 0.3s; }"
        ".opponent .die { background:#ff6b6b; color:white; box-shadow:0 10px 20px rgba(0,0,0,0.3), inset 0 -4px 0 rgba(255,255,255,0.2); }"
        ".rolling { animation: roll 0.3s infinite; }"
        "@keyframes roll { 0%{transform:rotate(0deg) scale(1);} 50%{transform:rotate(180deg) scale(1.1);} 100%{transform:rotate(360deg) scale(1);} }"
        "button { background:#ffd700; border:none; color:#1e1e2f; font-size:1.5rem; padding:15px 50px; border-radius:50px; font-weight:bold; cursor:pointer; transition:all 0.3s; margin:1.5rem 0; }"
        "button:hover { transform:scale(1.05); box-shadow:0 0 30px #ffd700; }"
        ".result { font-size:1.8rem; font-weight:bold; margin-top:1rem; min-height:3rem; }"
        "</style>"
        "</head>"
        "<body>"
        "<div class=\"container\">"
        "<h1>🎲 骰子大作战</h1>"
        "<div class=\"player-name\">⚔️ 玩家：" + username + "</div>"
        "<div style=\"display:flex; justify-content:center; gap:3rem;\">"
        "<div>"
        "<h3>你</h3>"
        "<div class=\"dice-container player\">"
        "<div class=\"die\" id=\"pdie1\">-</div>"
        "<div class=\"die\" id=\"pdie2\">-</div>"
        "</div>"
        "</div>"
        "<div>"
        "<h3>对手</h3>"
        "<div class=\"dice-container opponent\">"
        "<div class=\"die\" id=\"odie1\">-</div>"
        "<div class=\"die\" id=\"odie2\">-</div>"
        "</div>"
        "</div>"
        "</div>"
        "<button id=\"rollBtn\">🎲 掷骰子</button>"
        "<div class=\"result\" id=\"resultText\"></div>"
        "<div id=\"rewardDiv\" style=\"margin-top: 1.5rem;\"></div>"
        "<div id=\"punishDiv\" style=\"margin-top: 1rem;\"></div>"
        "</div>"
        "<script>"
        // 骰子点数对应的 Unicode 字符（⚀=1, ⚁=2, ⚂=3, ⚃=4, ⚄=5, ⚅=6）
        "const diceSymbols = ['', '⚀', '⚁', '⚂', '⚃', '⚄', '⚅'];"
        "document.getElementById('rollBtn').addEventListener('click', async () => {"
        "  const btn = document.getElementById('rollBtn');"
        "  btn.disabled = true;"
        "  document.querySelectorAll('.die').forEach(d => d.classList.add('rolling'));"
        "  try {"
        "    let resp = await fetch('/api/roll');"
        "    let data = await resp.json();"
        "    setTimeout(() => {"
        "      document.querySelectorAll('.die').forEach(d => d.classList.remove('rolling'));"
        // 用骰子符号替换数字
        "      document.getElementById('pdie1').textContent = diceSymbols[data.die1];"
        "      document.getElementById('pdie2').textContent = diceSymbols[data.die2];"
        "      document.getElementById('odie1').textContent = diceSymbols[data.opp_die1];"
        "      document.getElementById('odie2').textContent = diceSymbols[data.opp_die2];"
        "      document.getElementById('resultText').textContent = data.result;"
        "      let rewardDiv = document.getElementById('rewardDiv');"
        "      if (data.reward_url) {"
        "        rewardDiv.innerHTML = '<a href=\"' + data.reward_url + '\" style=\"color: #ffd700; font-size:1.3rem; text-decoration:none; background:rgba(255,255,255,0.15); padding:10px 35px; border-radius:30px;\">🎁 领取奖励</a>';"
        "      } else {"
        "        rewardDiv.innerHTML = '';"
        "      }"
        "      let punishDiv = document.getElementById('punishDiv');"
        "      if (data.punish_url) {"
        "        punishDiv.innerHTML = '<a href=\"' + data.punish_url + '\" style=\"color: #ff6b6b; font-size:1.3rem; text-decoration:none; background:rgba(255,255,255,0.15); padding:10px 35px; border-radius:30px;\">😈 接受惩罚</a>';"
        "      } else {"
        "        punishDiv.innerHTML = '';"
        "      }"
        "      btn.disabled = false;"
        "    }, 1000);" // 延长至1秒
        "  } catch(e) {"
        "    document.querySelectorAll('.die').forEach(d => d.classList.remove('rolling'));"
        "    btn.disabled = false;"
        "  }"
        "});"
        "</script>"
        "</body>"
        "</html>";
    return html;
}
void Login(HttpServer &server, HttpRequest &reqs, HttpResponse &resp)
{
    // for test
    // sleep(30);
    if(reqs.HasParam("nickname") == false)
    {
        resp._statu = 409; //Conflict
        server.ErrorHandler(resp);
        return;
    }
    std::string name = reqs.GetParam("nickname");
    std::string html = generateRainbowPage(name);
    resp.SetContent(html, ".html");
}
void PutFile(HttpServer &server, HttpRequest &req, HttpResponse &rsp) 
{
    std::string pathname = "./wwwroot" + req._path;
    Util::WriteToFile(pathname, req._body);
}
void Game(HttpServer &server, HttpRequest &reqs, HttpResponse &resp)
{
    if(reqs.HasParam("username") == false)
    {
        resp._statu = 409; //Conflict
        server.ErrorHandler(resp);
        return;
    }
    std::string name = reqs.GetParam("username");
    std::string html = generateGamePage(name);
    resp.SetContent(html, ".html");
}

#include <signal.h>
#include <execinfo.h> 
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

#include "Daemon.hpp"
int main()
{
    // 设置输出重定向
    int log_fd = open("./log/log.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd < 0) exit(1);

    dup2(log_fd, STDOUT_FILENO);
    dup2(log_fd, STDERR_FILENO);
    close(log_fd);  // 关闭多余的文件描述符

    // 同样，设置无缓冲
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    
    struct sigaction sa;
    sa.sa_handler = crashHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;  // 处理一次后自动恢复默认，防止死循环
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGBUS,  &sa, nullptr);
    sigaction(SIGFPE,  &sa, nullptr);

    Use_File_Log_Strategy;
    DaemonModule::Daemon(1, 0);

    HttpServer server(8080);

    server.SetBaseDir("./wwwroot");
    server.SetCount(3);

    server.Put("/1234.txt", PutFile);
    // /login?username=waspqj&password=
    server.Get("/register", [](HttpServer &server, HttpRequest &reqs, HttpResponse &resp){Login(server, reqs, resp); });
    server.Get("/login", [](HttpServer &server, HttpRequest &reqs, HttpResponse &resp){Game(server, reqs, resp); });
    server.Get("/api/roll", [](HttpServer &server, HttpRequest& req, HttpResponse& resp) {
        int die1 = rand() % 6 + 1;
        int die2 = rand() % 6 + 1;
        int opponentDie1 = rand() % 6 + 1;
        int opponentDie2 = rand() % 6 + 1;
        std::string result;
        std::string rewardUrl;
        std::string punishUrl;   // 新增
        int playerTotal = die1 + die2;
        int opponentTotal = opponentDie1 + opponentDie2;
        if (playerTotal > opponentTotal) {
            result = "🎉 你赢了！";
            rewardUrl = "/image/star.html";
            punishUrl = "";
        } else if (playerTotal < opponentTotal) {
            result = "😞 你输了...";
            rewardUrl = "";
            punishUrl = "/punish";   // 惩罚页路径，改成你自己的
        } else {
            result = "🤝 平局！";
            rewardUrl = "";
            punishUrl = "";
        }

        char json[512];
        snprintf(json, sizeof(json),
            "{\"die1\":%d,\"die2\":%d,\"opp_die1\":%d,\"opp_die2\":%d,"
            "\"result\":\"%s\",\"reward_url\":\"%s\",\"punish_url\":\"%s\"}",
            die1, die2, opponentDie1, opponentDie2, 
            result.c_str(), rewardUrl.c_str(), punishUrl.c_str());
        resp.SetContent(json, "application/json");
    });
    server.Get("/punish", [](HttpServer &server, HttpRequest& req, HttpResponse& resp) {
        resp.SetContent(generatePunishPage(), "text/html; charset=UTF-8");
    });

    server.Start();
    
    return 0;
}