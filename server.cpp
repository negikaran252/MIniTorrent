#include <bits/stdc++.h>
#include <unistd.h>
#include <stdio.h>
#include <map>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string>
#include <thread>
#include <filesystem>
#include <sys/stat.h>
#include<fstream>
#include <experimental/filesystem>
using namespace std;

// struct group1
// {
//     string group_id;
//     string group_owner;
//     set<string> group_mem;
// };
map<string, set<string>> groupfiles;
map<string, string> filename_filepath;
map<string, bool> online;
map<string, string> users;
map<string, set<string>> groups;
map<string, string> groupowner;
map<int, string> fd_user;
map<string,string> user_port;
map<string, set<string>> grouprequests;
map<string,string> file_owner;

int TPort;

vector<string> string_to_vector(string str)
{
    vector<string> vec;
    stringstream ss(str);
    string word;
    while (ss >> word)
    {
        vec.push_back(word);
    }
    return vec;
}

void get_tracker_details(string str)
{
	string line;
	ifstream readFile(str);
	while (getline (readFile, line));
	readFile.close();
	vector<string> temp_array;
    stringstream sst(line);
    while (sst.good())
    {
        string token;
        getline(sst, token, ':');
        temp_array.push_back(token);
    }
    TPort = stoi(temp_array[1]);
}

string getfileName(string filepath)
{
    int index = filepath.find_last_of("/\\");
    string file_name = filepath.substr(index + 1);
    return file_name;
}

void send_file(string file_path, int clifd)
{
    //cout << file_path << endl;
    char buff[100000];
    // fstream output_file(file_path.c_str(), ios::binary);
    // output_file.put('\0');

    int filefd = open(file_path.c_str(), O_RDONLY);
    if (filefd == -1)
    {
        cout << "error opening file";
        exit(EXIT_FAILURE);
    }
    ifstream in_file(file_path.c_str(), ios::binary);
    in_file.seekg(0, ios::end);
    long long int filesize = in_file.tellg();
    // cout<<filesize<<endl;
    string flsz=to_string(filesize);
    // cout<<flsz.c_str()<<endl;
    // cout<<sizeof(flsz.c_str())<<endl;
    write(clifd,flsz.c_str(),sizeof(flsz.c_str())+1);
    read(clifd,buff,sizeof(buff));
    bzero(buff, sizeof(buff));
    for (int byte_read = read(filefd, buff, sizeof(buff)); byte_read > 0; byte_read = read(filefd, buff, sizeof(buff)))
    {
        // cout<<buff<<" "<<byte_read;
        if(byte_read==0)
        {
            
            break;
        }
        write(clifd, buff, byte_read);
        bzero(buff,sizeof(buff));
        // read(clifd,buff,sizeof(buff));
        // bzero(buff, 7000);
    }
    //write(clifd,"done",4);

    //cout<<"hello"<<endl;
    close(filefd);
}

string do_task(string client_command, int sfd)
{
    vector<string> client_command_array;
    client_command_array.clear();
    stringstream iss(client_command);
    string word;
    while (iss >> word)
        client_command_array.push_back(word);
    string command = client_command_array[0];
    if (command == "create_user")
    {
        int nargs = client_command_array.size();
        if (nargs != 3)
        {
            return "Some arguments are missing";
        }
        string usrid = client_command_array[1];
        string pass = client_command_array[2];
        if (users.find(usrid) == users.end())
        {
            users[usrid] = pass;
            string s = "You have registered successfully.";
            return s;
        }
        else
        {
            string s = "User Already exists.";
            return s;
        }
    }
    else if (command == "login")
    {
        int nargs = client_command_array.size();
        if (nargs != 4)
        {
            return "Some arguments are missing";
        }
        string usrid = client_command_array[1];
        string pass = client_command_array[2];
        string p=client_command_array[3];
        if (users.find(usrid) != users.end() && users[usrid] == pass)
        {
            string s = "You have successfully logged In.";
            fd_user[sfd] = usrid;
            online[usrid] = true;
            user_port[usrid]=p;
            return s;
        }
        else
        {
            string s = "Please check your credentials.";
            return s;
        }
    }
    else if (command == "create_group")
    {
        int nargs = client_command_array.size();
        if (nargs != 2)
        {
            return "Some arguments are missing";
        }
        string userid = fd_user[sfd];
        if (online[userid] == false)
        {
            return "Login to your account.";
        }
        string groupid = client_command_array[1];
        if (groups.find(groupid) != groups.end())
        {
            string s = "Group already exists.";
            return s;
        }
        else
        {
            groups[groupid].insert(userid);
            groupowner[groupid] = userid;
            string s = "Group created successfully.";
            return s;
        }
    }
    else if (command == "join_group")
    {
        int nargs = client_command_array.size();
        if (nargs != 2)
        {
            return "Some arguments are missing";
        }
        string userid = fd_user[sfd];
        if (online[userid] == false)
        {
            return "Login to your account.";
        }
        string groupid = client_command_array[1];
        if (groups.find(groupid) != groups.end())
        {
            set<string> st = groups[groupid];
            set<string> s1 = grouprequests[groupid];
            if (st.find(userid) != st.end())
            {
                string s = "You are already a member of a group.";
                return s;
            }
            else if (s1.find(userid) != s1.end())
            {
                string s = "Your previous request is still pending.";
                return s;
            }
            else
            {
                grouprequests[groupid].insert(userid);
                string s = "Your request has been received.will inform you once the owner accepts your request.";
                return s;
            }
        }
        else
        {
            string s = "No such group exists.";
            return s;
        }
    }
    else if (command == "accept_request")
    {
        int nargs = client_command_array.size();
        if (nargs != 3)
        {
            return "Some arguments are missing";
        }
        string curr_user = fd_user[sfd];
        if (online[curr_user] == false)
        {
            return "Login to your account.";
        }
        string groupid = client_command_array[1];
        string userid = client_command_array[2];
        set<string> s1 = grouprequests[groupid];
        if (groupowner[groupid] == curr_user)
        {
            if (s1.find(userid) != s1.end())
            {
                grouprequests[groupid].erase(userid);
                groups[groupid].insert(userid);
                string s = userid + " has joined the group.";
                return s;
            }
            else
            {
                string s = userid + " has not requested to join the group.";
                return s;
            }
        }
        else
        {
            string s = "you are not admin of group.";
            return s;
        }
    }
    else if (command == "leave_group")
    {
        int nargs = client_command_array.size();
        if (nargs != 2)
        {
            return "Some arguments are missing";
        }
        string curr_user = fd_user[sfd];
        if (online[curr_user] == false)
        {
            return "Login to your account.";
        }
        string groupid = client_command_array[1];
        set<string> s1 = groups[groupid];
        if (s1.find(curr_user) != s1.end())
        {
            groups[groupid].erase(curr_user);
            if(groupowner[groupid]==curr_user)
            {
                groups.erase(groupid);
            }
            string s;
            s = "You have left the group.";
            return s;
        }
        else
        {
            string s;
            s = "You were already not in the group.";
            return s;
        }
    }
    else if (command == "list_groups")
    {
        string curr_user = fd_user[sfd];
        if (online[curr_user] == false)
        {
            return "Login to your account.";
        }
        string s = "";
        int sz = groups.size();
        s = to_string(sz) + " groups\n";
        for (auto itr = groups.begin(); itr != groups.end(); itr++)
        {
            string temp = itr->first;
            s = s + temp + "\n";
        }
        return s;
    }
    else if (command == "list_requests")
    {
        int nargs = client_command_array.size();
        if (nargs != 2)
        {
            return "Some arguments are missing";
        }
        string curr_user = fd_user[sfd];
        if (online[curr_user] == false)
        {
            return "Login to your account.";
        }
        string groupid = client_command_array[1];
        if(groupowner[groupid]!=curr_user)
        {
            return "You are not the owner of the group";
        }
        if (groups.find(groupid) != groups.end())
        {
            set<string> s1 = grouprequests[groupid];
            string s = "Request for group " + groupid;
            for (auto itr = s1.begin(); itr != s1.end(); itr++)
            {
                string temp = *itr;
                s = s + "\n" + temp;
            }
            return s;
        }
        else
        {
            string s = "Group doesn't exists.";
            return s;
        }
    }
    else if (command == "list_files")
    {
        int nargs = client_command_array.size();
        if (nargs != 2)
        {
            return "Some arguments are missing";
        }
        string curr_user = fd_user[sfd];
        if (online[curr_user] == false)
        {
            return "Login to your account.";
        }
        string groupid = client_command_array[1];
        if(groups[groupid].find(curr_user)==groups[groupid].end())
        {
            return "[-]YOU ARE NOT A MEMBER OF THE GROUP.";
        }
        if (groups.find(groupid) != groups.end())
        {
            string s = "SHARABLE FILES IN GROUP " + groupid+" ARE:";
            set<string> s1 = groupfiles[groupid];
            for (auto itr = s1.begin(); itr != s1.end(); itr++)
            {
                string temp = *itr;
                s = s + "\n" + temp;
            }
            return s;
        }
        else
        {
            return "[-]GROUP DOESN'T EXISTS.";
        }
    }
    else if (command == "logout")
    {
        string curr_user = fd_user[sfd];
        if (online[curr_user] == false)
        {
            return "Login to your account.";
        }
        string s;
        online[curr_user] = false;
        s = "You have logged out successfully.";
        return s;
    }
    else if (command == "upload_file")
    {
        int nargs = client_command_array.size();
        if (nargs != 3)
        {
            return "Some arguments are missing";
        }
        string curr_user = fd_user[sfd];
        if (online[curr_user] == false)
        {
            return "Login to your account.";
        }
        string groupid = client_command_array[2];
        string filepath = client_command_array[1];
        string file_name = getfileName(filepath);
        if (groups.find(groupid) == groups.end())
        {
            return "No such group exists.";
        }
        set<string> s1 = groups[groupid];
        if (s1.find(curr_user) != s1.end())
        {
            groupfiles[groupid].insert(file_name);
            filename_filepath[file_name] = filepath;
            file_owner[file_name]=curr_user;
            return "FILE UPLOADED SUCCESSFULLY.";
        }
        else
        {
            return "[-]YOU ARE NOT A MEMBER OF THE GROUP.";
        }
    }
    else if (command == "download_file")
    {
        if (client_command_array.size() != 4)
        {
            return "Some arguments are missing.";
        }
        string curr_user = fd_user[sfd];
        if (online[curr_user] == false)
        {
            return "Login to your account.";
        }
        string groupid = client_command_array[1];
        string filename = client_command_array[2];
        string destpath = client_command_array[3];
        if (groupfiles[groupid].find(filename) != groupfiles[groupid].end())
        {
            string file_path = filename_filepath[filename];
            string ret_str = "send__checkfile " + filename;
            return ret_str;
        }
        else
        {
            return "FILE DOESN'T EXIST IN GROUP.";
        }
    }
    else if(command=="stop_share")
    {
        if (client_command_array.size() != 3)
        {
            return "Some arguments are missing.";
        }
        string curr_user = fd_user[sfd];
        if (online[curr_user] == false)
        {
            return "Login to your account.";
        }
        string grpid=client_command_array[1];
        string filenm=client_command_array[2];
        if(groupfiles[grpid].find(filenm)!=groupfiles[grpid].end())
        {
            groupfiles[grpid].erase(filenm);
            return "[+] "+filenm+" STOPPED SHARING";
        }
        else
        {
            return "[-] FILE IS NOT IN GROUP.";
        }
    }
    else
    {
        return "[-]INVALID COMMAND";
    }
    return "[-]INVALID COMMAND";
}

void communicate(int new_sockfd)
{
    char buffer[2048];
    while (true)
    {
        bzero(buffer, 2048);
        int n = read(new_sockfd, buffer, 2048); //read data from client .
        if (n < 0)
        {
            cout << "Error on reading." << endl;
        }
        cout << "Client: " << buffer;
        string str = buffer;
        if(str=="quit\n")
        {
            cout<<"Do you want to close Tracker too"<<endl;
            close(new_sockfd);
            string check;
            cin>>check;
            if(check=="quit")
                exit(1);
            return;
        }
        bzero(buffer, 2048);
        string response = do_task(str, new_sockfd);
        bzero(buffer, 2048);
        vector<string> test_vec = string_to_vector(response);
        //cout<<test_vec[0];
        if (test_vec[0] == "send__checkfile")
        {
            string tenm=test_vec[1];
            string file_path = filename_filepath[tenm];
            send_file(file_path, new_sockfd);
            // int n = read(new_sockfd, buffer, 2048); 
            // cout<<buffer<<endl;
            // bzero(buffer,sizeof(buffer));
            read(new_sockfd, buffer, 2048);
            string uid=file_owner[tenm];
            string oport=user_port[uid];
            response = "Seeder: "+oport+"\n";
            strcpy(buffer, response.c_str());
            n = write(new_sockfd, buffer, strlen(buffer));
            
            // n = write(new_sockfd, buffer, strlen(buffer));
            // if (n < 0)
            // {
            //     cout << "error writing" << endl;
            // }
        }
        else
        {
            strcpy(buffer, response.c_str());
            n = write(new_sockfd, buffer, strlen(buffer));
            if (n < 0)
            {
                cout << "error writing" << endl;
            }
        }
    }
    close(new_sockfd);
}

int main(int argc, char *argv[])
{
    string trak=argv[1];
    get_tracker_details(trak);
    int server_sockfd, client_sockfd, portno;
    struct sockaddr_in server_addr, client_addr; //gives internet address.
    socklen_t client_len;                        // datatype in socket.h
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0)
    {
        cout << "Error opening socket." << endl;
    }
    else
    {
        cout << "Socket created successfully" << endl;
    }
    bzero((char *)&server_addr, sizeof(server_addr));
    char buffer[2048];
    portno = TPort;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; //assigning self IP address.
    server_addr.sin_port = htons(portno);
    int nbind = bind(server_sockfd, (sockaddr *)&server_addr, sizeof(server_addr));
    if (nbind < 0)
    {
        cout << "error binding";
    }
    else
    {
        cout << "binded successfully" << endl;
    }
    listen(server_sockfd, 10);
    while (true)
    {
        client_len = sizeof(client_addr);
        client_sockfd = accept(server_sockfd, (sockaddr *)&client_addr, &client_len);
        if (client_sockfd < 0)
        {
            cout << "error on accept" << endl;
        }
        else
        {
            cout << "accepted" << endl;
        }
        thread th(communicate, client_sockfd);
        th.detach();
        //communicate(client_sockfd);

        // bzero(buffer, 2048);
        // n = read(client_sockfd, buffer, 2048); //read data from client .
        // if (n < 0)
        // {
        //     cout << "Error on reading." << endl;
        // }
        // /*---------getting command from client----------------------*/

        // string client_command = buffer;
        // do_task(client_command);

        // bzero(buffer, 255);
        // fgets(buffer, 255, stdin);
        // n = write(client_sockfd, buffer, strlen(buffer));
        // if (n < 0)
        // {
        //     cout << "error writing" << endl;
        // }
        // char a[] = "bye";
        // int i = strncmp(a, buffer, 3);
        // if (i == 0)
        //     break;
    }
    close(server_sockfd);
    return 0; 
}