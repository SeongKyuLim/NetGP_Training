#include "Common.h"

#include <iostream>

bool IsLittleEndian();
bool IsBigEndian();

int main(int argc, char* argv[])
{
	using namespace std;

	if (IsLittleEndian())
		cout << "My Host is LittleEndian" << endl;
	if (IsBigEndian())
		cout << "My Host is BigEndian" << endl;

}

bool IsLittleEndian()
{
	WORD data = 0x1234;
	return data != htons(data);
}

bool IsBigEndian()
{
	WORD data = 0x1234;
	return data == htons(data);
}
