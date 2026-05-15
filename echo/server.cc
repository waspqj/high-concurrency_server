#include "Echo.hpp"

int main()
{
    Echo server(8081, 3);
    server.Start();
}