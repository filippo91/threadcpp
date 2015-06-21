// promesse.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"

#include <Windows.h>

#include <future>
#include <thread>
#include <iostream>
#include <list>
#include <string>
#include <fstream>
#include <algorithm>

#include "ThreadPool.h"

using namespace std;

const std::wstring cartellaOrigine = L"C:\\somePath\\*";
const std::string stringaDaCercare = "stringToSearch";

const std::wstring curDirectory = L".";
const std::wstring upLevelDirectory = L"..";

class FileFinder{
public:
	FileFinder(wstring nomeFile, std::string wanted){
		this->nomeFile = nomeFile;
		this->wanted = wanted;
	}
	std::wstring operator()(){
		std::ifstream in(nomeFile);
		std::string line;
		bool found;

		found = false;
		while(in){
			std::getline(in, line);
			auto it = std::search(line.begin(), line.end(),
				wanted.begin(), wanted.end());
			if(it != line.end()){
				found = true;
				break;
			}
		}

		in.close();

		return found ? nomeFile : L"";
	};
private:
	std::wstring nomeFile;
	std::string wanted; // :D
};

void exploreDir(std::wstring path, list<wstring>& files){
	WIN32_FIND_DATA data;
	DWORD error;
	std::wstring fileName;

	HANDLE hdir = FindFirstFile(path.data(), &data);

	if(hdir == INVALID_HANDLE_VALUE){
		DWORD error = GetLastError();
		if(error == ERROR_FILE_NOT_FOUND){
			std::cout << "Cartella di origine non trovata." << std::endl
			<< "Ricerca terminata." << std::endl;
		}else{
			std::cout << "Errore durante la ricerca della Cartella di origine." << std::endl
			<< "Ricerca terminata." << std::endl 
			<< "Identificatore Errore: " << error;
		}
		return;
	}else{
		std::wcout<< std::wstring(data.cFileName) <<std::endl;
	}

	while(FindNextFile(hdir, &data) != 0){
		fileName = std::wstring(data.cFileName);
		std::wcout<< fileName <<std::endl;
		if(data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY ){
			// skip '.' , '..' and anything different form a text file
			if(fileName.compare(curDirectory) == 0 || fileName.compare(upLevelDirectory) == 0){
				continue;
			}
			
			std::wstring newPath = path;
			newPath.erase(newPath.length()-1, 1);
			newPath.append(fileName).append(L"\\*");
			exploreDir(newPath, files);
			continue;
		}	

		std::wstring filePath = path;
		filePath.erase(filePath.length()-1, 1);
		filePath.append(fileName);
			
		files.push_back(fileName);
	}

	error = GetLastError();
		if(error == ERROR_NO_MORE_FILES){
			std::cout << "No More Files." << std::endl
			<< "Ricerca terminata." << std::endl;
		}else{
			std::cout << "Errore durante la ricerca." << std::endl
			<< "Ricerca terminata." << std::endl 
			<< "Identificatore Errore: " << error;
		}

		FindClose(hdir);
}



int _tmain(int argc, _TCHAR* argv[])
{
	ThreadPool<wstring> pool(10);
	list<shared_future<wstring>> l;
	list<wstring> files;
	cout << "SEARCH INTO DIRECTORIES" << endl << endl;

	//ls -R
	exploreDir(cartellaOrigine, files);

	//FileFinder circa una grep
	std::for_each(files.begin(), files.end(), [&pool, &l](wstring filePath){
		FileFinder fs(filePath, stringaDaCercare);
		packaged_task<std::wstring(void)> pt(fs);
		l.push_back(pt.get_future().share());
		pool.addTask(move(pt));
	});

	pool.close();

	cout << endl << "RESULTS" << endl << endl;

	std::for_each(l.begin(), l.end(), [](shared_future<wstring> f){
		wstring s = f.get();
		if(s.size() != 0)
			wcout << "trovata " << s << endl;
	});

	return 0;
}


