#include <arpa/inet.h> // for sockaddr_in, inet_addr
#include <cstdlib>     // for exit
#include <cstring>     // for memset
#include <iostream>
#include <sstream> // for std::ostringstream
#include <string>
#include <unistd.h> // for close

const int PORT = 18888;

void handle_client(int client_socket)
{
	char buffer[1024];
	std::memset(buffer, 0, sizeof(buffer));

	// 受信
	int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
	if (bytes_received < 0)
	{
		std::cerr << "Error receiving data" << std::endl;
		close(client_socket);
		return;
	}

	// 受信したリクエスト内容をログ出力
	std::cout << "=== Client Request ===" << std::endl;
	std::cout << buffer << std::endl;
	std::cout << "======================" << std::endl;

	// HTTPレスポンスボディ
	std::string body = "<html><body>hello</body></html>";

	// Content-Length を計算して文字列化
	std::ostringstream oss;
	oss << body.size();
	std::string content_length = oss.str();

	// HTTPレスポンス全体を組み立て
	std::string response = "HTTP/1.1 200 OK\r\n"
	                       "Content-Type: text/html\r\n"
	                       "Content-Length: " +
	                       content_length +
	                       "\r\n"
	                       "\r\n" +
	                       body;

	// レスポンスを送信
	int bytes_sent = send(client_socket, response.c_str(), response.size(), 0);
	if (bytes_sent < 0)
	{
		std::cerr << "Error sending data" << std::endl;
	}

	// クライアントソケットを閉じる
	close(client_socket);
	std::cout << "Client connection closed." << std::endl;
}

int main()
{
	// ソケット作成
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1)
	{
		std::cerr << "Failed to create socket" << std::endl;
		return EXIT_FAILURE;
	}

	// ソケットオプション設定
	int opt = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// サーバーアドレス設定
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	// ソケットにアドレスをバインド
	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		std::cerr << "Bind failed" << std::endl;
		close(server_socket);
		return EXIT_FAILURE;
	}

	// リスン開始
	if (listen(server_socket, 5) < 0)
	{
		std::cerr << "Listen failed" << std::endl;
		close(server_socket);
		return EXIT_FAILURE;
	}

	std::cout << "Server listening on port " << PORT << "..." << std::endl;

	while (true)
	{
		// クライアントからの接続を受け付け
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
		if (client_socket < 0)
		{
			std::cerr << "Accept failed" << std::endl;
			close(server_socket);
			return EXIT_FAILURE;
		}

		// 接続したクライアントの情報をログ出力
		std::cout << "Connection accepted from " << inet_ntoa(client_addr.sin_addr) << ":"
		          << ntohs(client_addr.sin_port) << std::endl;

		// クライアントを処理
		handle_client(client_socket);
	}

	// サーバーソケットを閉じる
	close(server_socket);

	return EXIT_SUCCESS;
}
