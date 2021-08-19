#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int main()
{
    string input_filename = "../Data/units-by-build-1v1-original.json";
    string output_filename = "../Data/units-by-build-1v1.json";

    ifstream read(input_filename, ios::binary);
    if (!read.is_open()) 
    {
        cout << "File " << input_filename << " was not opened!";
        return -1;
    }

    ofstream write(output_filename, ios::binary);
    if (!write.is_open())
    {
        cout << "File " << output_filename << " was not opened!";
        return -1;
    }

    read.seekg(2, ifstream::cur);
    string buffer;
    write << '[' << char(0xA);

    while (read.get() == 0x9) 
    {
        // for both players
        getline(read, buffer, '['); // get to the inner "units" div
        write << '\t' << buffer;

        getline(read, buffer, ']');
        write << '[' << buffer << "]},";

/*        if(x == 49851)
        {
            int pos = 0;
            cout << '[' << buffer << "]},";
            while (true)
            {
                char c = read.get();
                cout << (char)c;
                if (c == ']') 
                {
                    cout << read.tellg();
                    pos = read.tellg();
                    break;
                }
            }
            while (true)
            {
                cout << (char)read.get();
            }
            read.seekg(pos, ifstream::beg);
            while (true)
            {
                cout << (char)read.get();
            }
        } */

        //while (read.get() != ']');
/*      while (true)
        {
            char c = read.get();
            if (c == ']') break;
            cout << c;
        } */
        getline(read, buffer, ']');
        read.seekg(2, ifstream::cur);

        // for the second one
        getline(read, buffer, '['); // get to the inner "units" div
        write << buffer;

        getline(read, buffer, ']');
        write << '[' << buffer << "]}]}," << char(0xA);
        //while (read.get() != ']');
        getline(read, buffer, ']');
        read.seekg(5, ifstream::cur);
    }

    read.close();
    write << ']';
    write.close();
}