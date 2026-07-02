#ifndef UNTITLED_MAIN_H
#define UNTITLED_MAIN_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

#include "execucao.hpp"
class CPU;
class Tarefa;
class execucao;
using namespace std;

int quantum;
int num_cpus;
string type;
void readFile();
void inicializa_cpu(vector<CPU*>& f_cpu);

#endif //UNTITLED_MAIN_H