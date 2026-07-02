#ifdef _WIN32
#include <sys/stat.h>
// Define a função que o Linker está procurando
extern "C"
{
	int stat64i32(const char *path, struct _stat64i32 *buffer)
	{
		return _stat64i32(path, buffer);
	}
}
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>

#include "../include/interface.hpp"
#include "../include/execucao.hpp"

using namespace ProjetoSO;
using namespace std;

// Avisamos ao C++ que esta função existe no ficheiro file_dialog.cpp
string openFileDialog();

vector<Tarefa*> pega_tarefas(const string& caminho_arquivo) {
	vector<Tarefa*> tarefas;

	ifstream arquivo(caminho_arquivo);
	if (!arquivo.is_open()) 
	{
		// Fallback caso o ficheiro falhe ao abrir
		Tarefa* t1 = new Tarefa(1, {134, 206, 203, 255}, 0, 6, 5);
		Tarefa* t2 = new Tarefa(2, {190, 200, 209, 255}, 2, 4, 1);
		Tarefa* t3 = new Tarefa(3, {255, 40, 133, 255}, 0, 7, 5);
		Tarefa* t4 = new Tarefa(4, {255, 0, 69, 255}, 3, 2, 2);
		tarefas.push_back(t1);
		tarefas.push_back(t2);
		tarefas.push_back(t3);
		tarefas.push_back(t4);
		return tarefas;
	}
	cout << ">> Arquivo aberto com sucesso: " << caminho_arquivo << endl;

	string linha;
	// Pula a primeira linha (cabeçalho)
	getline(arquivo, linha);
	
	// Percorre todas as linhas de tarefas
	while (getline(arquivo, linha)) {
		if (linha.empty()) continue; 

		stringstream ss(linha);
		string temp;
		
		// 1. Lendo os 5 campos fixos iniciais da tarefa
		if (!getline(ss, temp, ';')) continue; 
		
		// --- NOVA LÓGICA DO ID ---
		// Extrai apenas os números (ignora letras como 'P' ou 'T' na frente)
		string id_str = "";
		for (char c : temp) {
			if (isdigit(c)) {
				id_str += c;
			}
		}
		if (id_str.empty()) continue; // Se não houver números, ignora a linha
		int id = stoi(id_str);
		// -------------------------

		if (!getline(ss, temp, ';')) continue; string hexColor = temp;
		if (!getline(ss, temp, ';')) continue; int t_ingresso = stoi(temp);
		if (!getline(ss, temp, ';')) continue; int t_execucao = stoi(temp);
		if (!getline(ss, temp, ';')) continue; int prioridade = stoi(temp);

		// Processando a cor hexadecimal (ex: "0000FF")
		unsigned char r = 0, g = 0, b = 0;
		if (hexColor.length() >= 6) {
			r = stoi(hexColor.substr(0, 2), nullptr, 16);
			g = stoi(hexColor.substr(2, 2), nullptr, 16);
			b = stoi(hexColor.substr(4, 2), nullptr, 16);
		}
		Color cor = {r, g, b, 255};

		Tarefa* nova_t = new Tarefa(id, cor, t_ingresso, t_execucao, prioridade);

		// 2. Captura o restante da linha para processar as Ações de forma robusta
		string resto_linha;
		getline(ss, resto_linha);
		
		size_t pos = 0;
		while (pos < resto_linha.size()) {
			// Ignora delimitadores comuns
			if (resto_linha[pos] == ';' || resto_linha[pos] == ' ' || resto_linha[pos] == '\r' || resto_linha[pos] == '\n') {
				pos++;
				continue;
			}
			
			// Detecta Mutex Lock (ML) ou Mutex Unlock (MU)
			if (resto_linha.compare(pos, 2, "ML") == 0 || resto_linha.compare(pos, 2, "MU") == 0) {
				string tipo = resto_linha.substr(pos, 2);
				pos += 2;
				
				// Captura o ID do Mutex (dígitos numéricos)
				string id_mutex_str = "";
				while (pos < resto_linha.size() && isdigit(resto_linha[pos])) {
					id_mutex_str += resto_linha[pos];
					pos++;
				}
				
				// Pula o caractere ':' se houver
				if (pos < resto_linha.size() && resto_linha[pos] == ':') pos++;
				
				// Captura o Instante de tempo
				string instante_str = "";
				while (pos < resto_linha.size() && isdigit(resto_linha[pos])) {
					instante_str += resto_linha[pos];
					pos++;
				}
				
				if (!id_mutex_str.empty() && !instante_str.empty()) {
					Tarefa::Acao acao;
					acao.tipo = tipo;
					acao.id_mutex = stoi(id_mutex_str);
					acao.instante = stoi(instante_str);
					acao.duracao = 0;
					acao.concluida = false;
					nova_t->addAcao(acao);
				}
			}
			// Detecta Entrada/Saída (IO)
			else if (resto_linha.compare(pos, 2, "IO") == 0) {
				string tipo = "IO";
				pos += 2;
				
				// Pula o caractere ':' se houver
				if (pos < resto_linha.size() && resto_linha[pos] == ':') pos++;
				
				// Captura o Instante de início do IO
				string instante_str = "";
				while (pos < resto_linha.size() && isdigit(resto_linha[pos])) {
					instante_str += resto_linha[pos];
					pos++;
				}
				
				// Pula o caractere '-' divisor
				if (pos < resto_linha.size() && resto_linha[pos] == '-') pos++;
				
				// Captura a Duração do IO
				string duracao_str = "";
				while (pos < resto_linha.size() && isdigit(resto_linha[pos])) {
					duracao_str += resto_linha[pos];
					pos++;
				}
				
				if (!instante_str.empty() && !duracao_str.empty()) {
					Tarefa::Acao acao;
					acao.tipo = tipo;
					acao.id_mutex = -1;
					acao.instante = stoi(instante_str);
					acao.duracao = stoi(duracao_str);
					acao.concluida = false;
					nova_t->addAcao(acao);
				}
			}
			else {
				// Avança caso encontre algum caractere inesperado (evita loops infinitos)
				pos++;
			}
		}

		tarefas.push_back(nova_t);
	}
	
	arquivo.close();
	cout << ">> Tarefas carregadas com sucesso!" << endl;
	return tarefas;
}

void readFile(int &quantum, int &num_cpus, string &type, const string& caminho_arquivo, int &alpha) 
{
	ifstream arquivo(caminho_arquivo);

	if (!arquivo.is_open()) {
		cerr << "Erro ao abrir o arquivo de configuracao (" << caminho_arquivo << ")!" << endl;
	}
	else {
		string linha;
		if (getline(arquivo, linha)) {
			stringstream ss(linha);
			string temp;
			
			// Lê de forma segura separando por ';'
			if (getline(ss, temp, ';')) type = temp;
			if (getline(ss, temp, ';')) quantum = stoi(temp);
			if (getline(ss, temp, ';')) num_cpus = stoi(temp);
			if (getline(ss, temp, ';') && !temp.empty() && temp != "\r" && temp != "\n") {alpha = stoi(temp);}
		}
		arquivo.close();
	}
}

int main()
{
	cout << ">> Aguardando selecao do arquivo no explorador..." << endl;
	string caminho_arquivo = openFileDialog();
	cout << ">> Caminho selecionado: " << caminho_arquivo << endl;

	Interface interface(1900, 1000, "Simulador de Escalonamento de Tarefas");
	SetTargetFPS(60);
	int quantum = 0, num_cpus = 0, alpha = 1;
	string type;
	
	readFile(quantum, num_cpus, type, caminho_arquivo, alpha);
	cout << "Tipo: " << type << " | Quantum: " << quantum << " | CPUs: " << num_cpus << " | Alpha: " << alpha <<endl;
	
	interface.setQuantum(quantum);
	interface.setCPUCount(num_cpus);
	interface.setSimulationType(type);
	
	interface.setTasks(pega_tarefas(caminho_arquivo));
	cout << ">> Tarefas definidas na interface" << endl;
	for (Tarefa* task : interface.getTasks()) 
	{
		cout << "Tarefa ID: " << task->getId() << endl;
	}

	while (!WindowShouldClose())
	{
		if (interface.draw())
		{ 
			if (Execucao::getInstance()->update())
			{
				cout << ">> Simulacao finalizada!" << endl;
				interface.setSimulationType(type);
				interface.setTasks(pega_tarefas(caminho_arquivo));
				interface.simulationFinished();
			}
		}
	}

	return 0;
}