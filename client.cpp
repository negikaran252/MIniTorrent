#include <bits/stdc++.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string>
#include <thread>
#include <netdb.h>
#include <filesystem>
#include <fstream>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;
string CIP;
int CPORT;
string Cdetails;
int TPort;
map<string, set<string>> filedowninfo;


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


void get_tracker_details(string str){
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


string maintain_requests(string client_command)
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
        string userid = client_command_array[1];
        string groupid = client_command_array[2];
        string p=client_command_array[3];
        int nargs = client_command_array.size();
        if (nargs != 4)
        {
            return "Some arguments are missing";
        }
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
    else if (command == "join_group")
    {
        int nargs = client_command_array.size();
        if (nargs != 2)
        {
            return "Some arguments are missing";
        }
        string userid = "usr";
        if (online[userid] == false)
        {
            return "Login to your account.";
        }
        string groupid = client_command_array[1];
        if (users.find(userid) != users.end() && users[userid] == "pass")
        {
            string s = "You have successfully logged In.";
            //fd_user[sfd] = usrid;
            online[userid] = true;
            user_port[userid]="p";
            return s;
        }
        else
        {
            string s = "Please check your credentials.";
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
        string curr_user =client_command_array[1];
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
        string curr_user = client_command_array[1];
        if (online[curr_user] == false)
        {
            return "Login to your account.";
        }
        string groupid = client_command_array[1];
        set<string> s1 = groups[groupid];
        if (s1.find(curr_user) != s1.end())
        {
            groups[groupid].erase(curr_user);
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
        string curr_user = client_command_array[1];
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
        string curr_user = client_command_array[1];
        if (online[curr_user] == false)
        {
            return "Login to your account.";
        }
        string groupid = client_command_array[1];
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
        string curr_user = client_command_array[1];
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
        string curr_user = client_command_array[1];
        if (online[curr_user] == false)
        {
            return "Login to your account.";
        }
        string s;
        online[curr_user] = false;
        s = "You have logged out successfully.";
        return s;
    }
    else if (command == "download_file")
    {
        if (client_command_array.size() != 4)
        {
            return "Some arguments are missing.";
        }
        string curr_user = client_command_array[1];
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
        string curr_user = client_command_array[1];
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
void peer_send_file(string file_path, int clifd)
{
    cout << file_path << endl;
    char buff[52000];
    fstream output_file(file_path.c_str(), ios::binary);
    output_file.put('\0');
    int filefd = open(file_path.c_str(), O_RDONLY);
    if (filefd == -1)
    {
        cout << "error opening file";
        exit(EXIT_FAILURE);
    }
    for (int byte_read = read(filefd, buff, sizeof(buff)); byte_read > 0; byte_read = read(filefd, buff, sizeof(buff)))
    {
        // cout<<buff<<" "<<byte_read;
        if(byte_read==0)
        {
            
            break;
        }
        write(clifd, buff, byte_read);
        bzero(buff, 52000);
        // read(clifd,buff,sizeof(buff));
        // bzero(buff, 7000);
    }
    //write(clifd,"done",4);

    cout<<"hello"<<endl;
    close(filefd);
}

void peer_server()
{
    int peer_server_sockfd, peer_client_sockfd, portno;
    struct sockaddr_in peer_server_addr, peer_client_addr; //gives internet address.
    //socklen_t client_len;                                  // datatype in socket.h
    peer_server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (peer_server_sockfd < 0)
    {
        cout << "Error opening socket." << endl;
    }
    else
    {
        cout << "Socket created successfully" << endl;
    }
    bzero((char *)&peer_server_addr, sizeof(peer_server_addr));
    char buffer[2048];
    portno = CPORT;
    peer_server_addr.sin_family = AF_INET;
    peer_server_addr.sin_addr.s_addr = INADDR_ANY; //assigning self IP address.
    peer_server_addr.sin_port = htons(portno);
    int nbind = bind(peer_server_sockfd, (sockaddr *)&peer_server_addr, sizeof(peer_server_addr));
    if (nbind < 0)
    {
        cout << "error binding";
    }
    else
    {
        cout << "binded successfully" << endl;
    }
    listen(peer_server_sockfd, 10);
    socklen_t client_len;
    client_len = sizeof(peer_client_addr);
    vector<thread> tids;
    while (true)
    {
        peer_client_sockfd = accept(peer_server_sockfd, (struct sockaddr *)&peer_client_addr, &client_len);
        if (peer_client_sockfd < 0)
            break;
        int port = (ntohs(peer_client_addr.sin_port));
        bzero(buffer, 2048);
        int n = read(peer_client_sockfd, buffer, 2048); //read data from client .
        if (n < 0)
        {
            cout << "Error on reading." << endl;
        }
        cout << "Client: " << buffer;
        string str = buffer;
        bzero(buffer, 2048);
        string response = maintain_requests(str);
        bzero(buffer, 2048);
        vector<string> test_vec = string_to_vector(response);
        //cout<<test_vec[0];
        if (test_vec[0] == "send__checkfile")
        {

            //send_file(test_vec[1], peer_client_sockfd);
            response = "Downloaded";
        }
        else
        {
            strcpy(buffer, response.c_str());
            n = write(peer_client_sockfd, buffer, strlen(buffer));
            if (n < 0)
            {
                cout << "error writing" << endl;
            }
        }
    }
    // for (auto itr = tids.begin(); itr != tids.end(); itr++)
    //     pthread_join(*itr, NULL);
}

void down_file(int sockfd, string destpath)
{
    char buff[100000];
    int dfilefd = open(destpath.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (dfilefd == -1)
    {
        cout << "**ERROR CREATING FILE**";
        cout << endl;
    }
    //cout << "Enter ";

    // fd_set set;
    // struct timeval timeout;
    // int rv;
    // FD_ZERO(&set);        /* clear the set */
    // FD_SET(sockfd, &set); /* add our file descriptor to the set */

    // timeout.tv_sec = 0;
    // timeout.tv_usec = 100;

    read(sockfd, buff, sizeof(buff));
    // cout<<sizeof(buff)<<endl;
    // cout<<buff<<endl;
    string strsz=buff;
    //cout<<"string: "<<strsz<<endl;
    long long int totalsize=stoll(strsz);
    //cout<<totalsize<<endl;
    bzero(buff, sizeof(buff));
    write(sockfd,"gotsize",7);
    for (int byte_read = read(sockfd, buff, sizeof(buff)); totalsize > 0;byte_read = read(sockfd, buff, sizeof(buff)))
    {
        totalsize=totalsize-byte_read;
        write(dfilefd, buff, byte_read);
        bzero(buff, sizeof(buff));
        if(totalsize<=0)
            break;
    }
    //cout << "Out" << endl;
    bzero(buff, sizeof(buff));
    close(dfilefd);
}
int main(int argc, char *argv[])
{
    get_tracker_details(argv[2]);
    Cdetails = argv[1];
    vector<string> v;
    stringstream sst(Cdetails);
    while (sst.good())
    {
        string token;
        getline(sst, token, ':');
        v.push_back(token);
    }
    CIP = v[0];
    string port = v[1];
    CPORT = atoi(port.c_str());
    int sockfd, portno, n;
    struct sockaddr_in server_address, server_address1;
    struct hostend *serv;
    portno = TPort; //port of tracker.
    char buffer[1024];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        cout << "error opening socket" << endl;
    }
    else
    {
        cout << "socket created successfully" << endl;
    }
    bzero((char *)&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(portno);

    /*
    server_address1.sin_family=AF_INET;
    server_address1.sin_addr.s_addr=INADDR_ANY;
    server_address1.sin_port=htons(2000);
    int nbind = bind(sockfd, (sockaddr *)&server_address1, sizeof(server_address1));
    if (nbind < 0)
    {
        cout << "error binding";
    }
    */

    // thread server_thread(peer_server);
    // server_thread.detach();
    int k = connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address));
    if (k < 0)
    {
        cout << "Error connecting" << endl;
    }
    else
    {
        cout << "Connected." << endl;
    }
    while (true)
    {

        bzero(buffer, 255);
        fgets(buffer, 255, stdin);
        string temp = buffer;
        vector<string> temp_vec = string_to_vector(temp);
        if (temp == "show_downloads\n")
        {
            bool flag = false;
            bzero(buffer, 255);
            for (auto it = filedowninfo.begin(); it != filedowninfo.end(); it++)
            {
                flag = true;
                string grp_id = it->first;
                set<string> tempset = it->second;
                for (auto iter = tempset.begin(); iter != tempset.end(); iter++)
                {
                    string filname = *iter;
                    cout << "[C] [" << grp_id << "] " << filname << endl;
                }
            }
            if (flag == false)
            {
                cout << "[-] YOU HAVE NOT DOWNLOADED ANY FILE YET." << endl;
                continue;
            }
            cout << "D(Downloading), C(Complete)" << endl;
            continue;
        }
        if (temp_vec[0] == "login")
        {
            bzero(buffer, 255);
            temp.pop_back();
            string resp = temp + " " + CIP + ":" + port;
            strcpy(buffer, resp.c_str());
            buffer[strlen(buffer)] = '\n';
        }
        n = write(sockfd, buffer, 255);
        if (n < 0)
        {
            cout << "error sending" << endl;
        }
        if (temp == "quit\n")
        {
            break;
        }
        string str = buffer;
        bzero(buffer, 255);
        vector<string> comm_vec = string_to_vector(str);
        // cout<<comm_vec[0]<<endl;
        if (comm_vec[0] == "download_file")
        {
            cout << "DOWNLOAD STARTED." << endl;
            down_file(sockfd, comm_vec[3]);
            cout << "DOWNLOAD COMPLETED." << endl;
            n = write(sockfd, "thanks", 6);
            string grpid = comm_vec[1];
            string filename = comm_vec[2];
            if (filedowninfo.find(grpid) != filedowninfo.end())
            {
                set<string> s1 = filedowninfo[grpid];
                s1.insert(filename);
                filedowninfo[grpid] = s1;
            }
            else
            {
                filedowninfo[grpid].insert(filename);
            }
            // continue;
        }
        n = read(sockfd, buffer, 255);
        if (n < 0)
        {
            cout << "error" << endl;
        }
        else
        {
            cout << buffer;
            cout << endl;
        }
    }
    // char a[]="bye";
    //     int i=strncmp(a,buffer,3);
    //     if(i==0)
    //         break;
    close(sockfd);
    return 0;
}