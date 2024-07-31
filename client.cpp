#include <arpa/inet.h>
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h>     /* read, write, close */

#include <iostream>
#include <string>

#include "helpers.hpp"
#include "json.hpp"
#include "requests.hpp"

using namespace std;

int sockFD;
string sessionCookie;
string token;

string extractJsonErr(char *str) {
  char *jsonStr = basic_extract_json_response(str);
  if (!jsonStr){
    return NULL;
  } 
  return nlohmann::json::parse(jsonStr).value("error", "unkown");
}

int printResponseMsg(char *msg, const char *str) {
  int code = getStatusCode(msg);
  if (code == 200 || code == 201) {
    cout << str << "\n";
  }
  else {
    cout << extractJsonErr(msg) << "\n\n";
  }
  return code;
}

void registerCommand() {
  sockFD = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

  string username, password;

  cout << "username=";
  getline(cin, username);

  cout << "password=";
  getline(cin, password);

  nlohmann::json credentials = {{"username", username}, {"password", password}};
  string creds = credentials.dump();
  char* str = new char[creds.length() + 1];
  strcpy(str, creds.c_str());
  char *msg = compute_post_request(HOST, REGISTER, APP_JSON,
                                  nullptr, nullptr, 0, &str, 1, nullptr, 0);
  send_to_server(sockFD, msg);
  delete[](str);
  free(msg);

  char *response = receive_from_server(sockFD);
  printResponseMsg(response, "Registration completed!\n");
  free(response);

  close_connection(sockFD);
}


void loginCommand() {
  sockFD = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

  string username, password;

  cout << "username=";
  getline(cin, username);

  cout << "password=";
  getline(cin, password);

  nlohmann::json credentials = {{"username", username}, {"password", password}};
  string creds = credentials.dump();
  char* str = new char[creds.length() + 1];
  strcpy(str, creds.c_str());

  char *msg = compute_post_request(HOST, LOGIN, APP_JSON,
                                  nullptr, nullptr, 0, &str, 1, nullptr, 0);
  delete[](str);

  send_to_server(sockFD, msg);
  free(msg);

  char *response = receive_from_server(sockFD);
  int code = printResponseMsg(response, "Login completed!\n");
  if (code == 200 || code == 201){
    sessionCookie = getSessionCookie(response);
  }
  free(response);
  
  
  close_connection(sockFD);
}

void enterCommand() {
  sockFD = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

  char* cookie = new char[sessionCookie.length() + 1];
  strcpy(cookie, sessionCookie.c_str());

  char *msg = compute_get_request(HOST, ACCESS, nullptr, nullptr, nullptr, 0, &cookie, 1);
  delete[](cookie);

  send_to_server(sockFD, msg);
  free(msg);

  char *response = receive_from_server(sockFD);

  int code = printResponseMsg(response, "Enter library completed!\n");
  if (code == 200 || code == 201) {
    char *jsonStr = basic_extract_json_response(response);
    free(response);

    nlohmann::json json = nlohmann::json::parse(jsonStr);
    token = "Bearer " + json.value("token", "unknown");
  }
  
  
  close_connection(sockFD);
}

void addBook() {
    sockFD = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    string title, author, genre, publisher, page_count;
    cout << "title=";
    getline(cin, title);

    cout << "author=";
    getline(cin, author);

    cout << "genre=";
    getline(cin, genre);

    cout << "publisher=";
    getline(cin, publisher);

    cout << "page_count=";
    getline(cin, page_count);

    nlohmann::json json = {
        {"title", title},
        {"author", author},
        {"genre", genre},
        {"publisher", publisher},
        {"page_count", page_count}
    };

    try {
      stoi(page_count);
    } catch (const std::invalid_argument& e) {
      std::cerr << "Invalid argument, must be an integer!" << "\n";
    }

    string jsonStr = json.dump();
    char* jsonStrCpy = new char[jsonStr.length() + 1];
    strcpy(jsonStrCpy, jsonStr.c_str());

    string header_names = "Authorization";
    char* headers = new char[token.length() + 1];
    strcpy(headers, token.c_str());
    
    char* headerNamesCopy = new char[header_names.length() + 1];
    strcpy(headerNamesCopy, header_names.c_str());

    char *msg = compute_post_request(HOST, BOOKS, APP_JSON, 
                                         &headers, &headerNamesCopy, 
                                         1, &jsonStrCpy, 1, nullptr, 0);
    delete[] headers;
    delete[] headerNamesCopy;
    delete[] jsonStrCpy;

    send_to_server(sockFD, msg);
    free(msg);

    char *response = receive_from_server(sockFD);
    printResponseMsg(response, "Add book completed!\n");
    free(response);

    close_connection(sockFD);
}

void getBooks() {
    sockFD = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    char* headers = new char[token.length() + 1];
    strcpy(headers, token.c_str());
    string header_names = "Authorization";
    char* headerNamesCopy = new char[header_names.length() + 1];
    strcpy(headerNamesCopy, header_names.c_str());

    char *msg = compute_get_request(HOST, BOOKS, nullptr, &headers,
                                    &headerNamesCopy, 1, nullptr, 0);
    delete[] headers;
    delete[] headerNamesCopy;

    send_to_server(sockFD, msg);
    free(msg);
    char *response = receive_from_server(sockFD);

    int code = getStatusCode(response);
    if (code == 200 || code == 201) {
        char *jsonStr = basic_extract_json_response(response);
        if (!jsonStr) {
            cout << "Empty library!\n\n";
        } else {
            nlohmann::json json = nlohmann::json::parse(jsonStr);
            for (auto elem : json) {
                cout << "id: " << elem["id"] << '\n'
                     << "title: " << elem["title"] << "\n\n";
            }
        }
    } else {
        cout << extractJsonErr(response) << "\n\n";
    }
    free(response);

    close_connection(sockFD);
}


void getBook() {
  sockFD = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

  string id;
  cout << "id=";
  getline(cin, id);

  string header_names = "Authorization";
  char* headers = new char[token.length() + 1];
  strcpy(headers, token.c_str());

  char* headerNamesCopy = new char[header_names.length() + 1];
  strcpy(headerNamesCopy, header_names.c_str());

  string urlStr = BOOKS;
  urlStr.append("/");
  urlStr.append(id);
  char *msg = compute_get_request(HOST, urlStr.c_str(), nullptr,
                                 &headers, &headerNamesCopy, 1, nullptr, 0);
  delete[] headerNamesCopy;
  delete[] headers;

  send_to_server(sockFD, msg);
  free(msg);

  char *response = receive_from_server(sockFD);
  int statusCode = getStatusCode(response);
  if (statusCode == 200 || statusCode == 201) {
    char *jsonStr = basic_extract_json_response(response);
    nlohmann::json json = nlohmann::json::parse(jsonStr);
    for (const auto& [key, value] : json.items()) {
        if (key == "id" || key == "title" || key == "author" || key == "publisher" || key == "genre" || key == "page_count") {
            if (value.is_string()) {
              string strValue = value.get<string>();
                
              strValue.erase(remove(strValue.begin(), strValue.end(), '\"'), strValue.end());
              cout << key << ": " << strValue << '\n';
            } else {
              cout << key << ": " << value << '\n';
            }
        }
    }
  } else {
    cout << extractJsonErr(response) << "\n\n";
  }
  free(response);

  close_connection(sockFD);
}


void deleteBook() {
    sockFD = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    string id;

    cout << "id=";
    getline(cin, id);

    char* headers = new char[token.length() + 1];
    strcpy(headers, token.c_str());

    string header_names = "Authorization";
    char* headerNamesCopy = new char[header_names.length() + 1];
    std::strcpy(headerNamesCopy, header_names.c_str());

    string urlStr = BOOKS;
    urlStr.append("/");
    urlStr.append(id);

    char *msg = compute_delete_request(HOST, urlStr.c_str(), nullptr, 
           &headers, &headerNamesCopy, 
           1, nullptr, 0);
    delete[] headerNamesCopy;
    delete[] headers;

    send_to_server(sockFD, msg);
    free(msg);

    char *response = receive_from_server(sockFD);
    printResponseMsg(response, "Delete book completed!\n");
    free(response);

    close_connection(sockFD);
}

void logoutCommand() {
  sockFD = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

  char* cookie = new char[sessionCookie.length() + 1];
  strcpy(cookie, sessionCookie.c_str());

  char *msg = compute_get_request(HOST, LOGOUT, nullptr, nullptr, nullptr, 0, &cookie, 1);
  delete[](cookie);

  send_to_server(sockFD, msg);
  free(msg);

  char *response = receive_from_server(sockFD);
  int code = printResponseMsg(response, "Logout completed!\n");
  free(response);

  if (code == 200 || code == 201) {
    sessionCookie.clear();
    token.clear();
  }

  close_connection(sockFD);
}

int main(int argc, char *argv[]) {
  string input;

  while (true) {
    getline(cin, input);

    if (input == "exit") {
      break;
    }
    else if (input == "register") {
      if (sessionCookie.empty())
        registerCommand();
      else
        cout << "Already logged in!\n\n";
    }
     else if (input == "login") {
      if (sessionCookie.empty())
        loginCommand();
      else
        cout << "Already logged in!\n\n";
    }
    else if (input == "logout") {
      logoutCommand();
    }
    else if (input == "enter_library") {
      enterCommand();
    }
    else if (input == "get_books") {
      if (sessionCookie.empty())
        cout << "Not logged in!\n\n";
      else
        getBooks();
    }
    else if (input == "get_book") {
      if (sessionCookie.empty())
        cout << "Not logged in!\n\n";
      else
        getBook();
    }
    else if (input == "add_book") {
      if (sessionCookie.empty())
        cout << "Not logged in!\n\n";
      else
        addBook();
    }
    else if (input == "delete_book") {
      if (sessionCookie.empty())
        cout << "Not logged in!\n\n";
      else
        deleteBook();
    } else {
      cout << "UNKNOWN COMMAND!\n\n";
    }
  }

  return 0;
}
