#include "../include/execucao.hpp"

#include <iostream>
#include <string>
#include <algorithm>

using namespace ProjetoSO;
using namespace std;

Execucao* Execucao::getInstance()
{
	static Execucao instance;
	return &instance;
}

std::unique_ptr<Escalonador> Execucao::criarEscalonador(const std::string& type, unsigned int quantum)
{
	if (type == "FCFS")      return std::make_unique<FCFSEscalonador>();
	if (type == "SJF")       return std::make_unique<SJFEscalonador>();
	if (type == "SRTF")      return std::make_unique<SRTFEscalonador>();
	if (type == "PRIOP")     return std::make_unique<PRIOPEscalonador>();
	if (type == "PRIOPENV") return std::make_unique<PRIOPEnvelhecimentoEscalonador>(this->alpha);
	if (type == "PRIOD")     return std::make_unique<PRIOdEscalonador>();
	if (type == "RR")        return std::make_unique<RoundRobinEscalonador>();
	return nullptr;
}

void Execucao::init(std::string type, std::vector<Tarefa*> todas_tarefas, unsigned int quantum, unsigned int cpuCount, unsigned int alpha)
{
	if (cpu_list.size() > 0)
		for (CPU* cpu : cpu_list)
			delete cpu;
	this->cpu_list.clear();
	this->todas_tarefas.clear();
	this->type = type;
	this->escalonador = criarEscalonador(type, quantum);
	this->todas_tarefas = todas_tarefas;
	this->quantum = quantum;
	this->alpha = alpha;
	tarefas.clear();
	finalizadas.clear();
	for (unsigned int i = 0; i < cpuCount; i++)
		cpu_list.push_back(new CPU(i));
	relogio = 0;
	for (Tarefa* t : todas_tarefas) {
		t->resetEventos();
	}
	cout << ">> Execucao inicializada com " << cpuCount << " CPUs, quantum de " << quantum << "s, tipo de escalonamento " << type << " e " << todas_tarefas.size() << " tarefas." << endl;
}

void Execucao::entra_tarefa() 
{
	for (Tarefa* t : todas_tarefas) {
		if (t->getTempoIngresso() == relogio) {
			tarefas.push_back(t);
			t->registrarEvento(Tarefa::TaskState::Ready, relogio);
		}
	}
}

void Execucao::finalizar_tarefa(Tarefa* t) 
{
	if (std::any_of(cpu_list.begin(), cpu_list.end(), [t](CPU* c){ return c->getTarefa() == t; })) {
		for (CPU* c : cpu_list) {
			if (c->getTarefa() == t) {
				c->setTarefa(nullptr);
			}
		}
	}
	else if (std::any_of(tarefas.begin(), tarefas.end(), [t](Tarefa* tarefa){ return tarefa == t; })) {
		for (size_t i = 0; i < tarefas.size(); i++) {
			if (t == tarefas[i]) {
				tarefas.erase(tarefas.begin() + i);
			}
		}
	}
}

void Execucao::mandar_fila_prontos(Tarefa* t){
	for (CPU* cpu : cpu_list) {
		if (cpu->getTarefa() == t) {
			cpu->setTarefa(nullptr);
			tarefas.push_back(t);
			break;
		}
	}
}

void Execucao::mandar_cpu(Tarefa* t) {
	for (CPU* cpu : cpu_list) {
		if (cpu->getTarefa() == nullptr) {
			cpu->setTarefa(t);
			break;
		}
	}
}

bool Execucao::getCPUVazio() {
	for (CPU* cpu : cpu_list) {
		if (!cpu->isOcupado()) return true;
	}
	return false;
}

void Execucao::automatico() 
{
	while (!update()) {	}
}

bool Execucao::update() 
{
	entra_tarefa();
	unidade_tempo();

	if (!escalonador)
		return true;

	escalonador->executar(cpu_list, tarefas, relogio, quantum);

	bool terminou = tarefas.empty();
	for (CPU* cpu : cpu_list) {
		if (cpu->getTarefa() != nullptr) terminou = false;
	}

	relogio++;
	for (CPU* cpu : cpu_list)
		cpu->executando();

	return terminou;
}

void Execucao::unidade_tempo() 
{
    // 1. Desconta o tempo de I/O de todas as tarefas suspensas globalmente
    atualizarIO();

    for (CPU* cpu : cpu_list) {
        Tarefa* t = cpu->getTarefa();

        if (t != nullptr) {
            // 2. Processa as interrupções/ações ANTES do avanço temporal da CPU
            processarAcoes(cpu, t);

            // Verifica se a tarefa continua na CPU após o processamento das ações
            t = cpu->getTarefa();
            if (t != nullptr) {
                t->executarUnidade();
                t->incrementaTempoExecutado(); // Incrementa o contador interno relativo

                if (t->isConcluida()) {
                    cpu->liberarCPU();
                    t->registrarEvento(Tarefa::TaskState::Finished, relogio);
                    finalizadas.push_back(t);
                }
            }
        }
        else {
            cpu->unidadeDesligado();
        }
    }
}
void Execucao::processarAcoes(CPU* cpu, Tarefa* t)
{
    for (auto& acao : t->getAcoes()) {
        // Se o instante relativo de execução da tarefa bate com a ação programada
        if (!acao.concluida && acao.instante == t->getTempoExecutado()) {
            acao.concluida = true;

            if (acao.tipo == "ML") { // Mutex Lock
                if (mutex_donos[acao.id_mutex] == nullptr) {
                    mutex_donos[acao.id_mutex] = t; // Adquire o Mutex livre
                } else {
                    // Mutex ocupado: desaloca da CPU imediatamente e joga na fila de bloqueados
                    cpu->liberarCPU();
                    t->registrarEvento(Tarefa::TaskState::BlockedMutex, relogio);
                    mutex_filas[acao.id_mutex].push(t);
                    break; 
                }
            }
            else if (acao.tipo == "MU") { // Mutex Unlock
                if (mutex_donos[acao.id_mutex] == t) {
                    mutex_donos[acao.id_mutex] = nullptr;
                    // Se houver tarefas esperando por este Mutex, acorda a primeira
                    if (!mutex_filas[acao.id_mutex].empty()) {
                        Tarefa* proxima = mutex_filas[acao.id_mutex].front();
                        mutex_filas[acao.id_mutex].pop();
                        mutex_donos[acao.id_mutex] = proxima;
                        
                        proxima->registrarEvento(Tarefa::TaskState::Ready, relogio);
                        tarefas.push_back(proxima); // Retorna à fila de prontos do SO
                    }
                }
            }
            else if (acao.tipo == "IO") { // Operação de E/S
                cpu->liberarCPU();
                t->registrarEvento(Tarefa::TaskState::BlockedIO, relogio);
                t->setTempoIORestante(acao.duracao);
                tarefas_em_io.push_back(t);
                break; 
            }
        }
    }
}

void Execucao::atualizarIO()
{
    for (auto it = tarefas_em_io.begin(); it != tarefas_em_io.end(); ) {
        Tarefa* t = *it;
        t->decrementarTempoIORestante();

        if (t->getTempoIORestante() <= 0) {
            // Conforme requisito 3.4: Ao terminar, gera IRQ e volta a ser Ready
            t->registrarEvento(Tarefa::TaskState::Ready, relogio);
            tarefas.push_back(t);
            it = tarefas_em_io.erase(it); // Remove da fila de E/S ativo
        } else {
            ++it;
        }
    }
}

void Execucao::quantum_tempo()
{
	for (unsigned int i = 0; i < quantum; i++)
	{
		relogio++;
		cout << "[RELOGIO: " << relogio << "s]" << endl;
		entra_tarefa();

        if (escalonador) {
			escalonador->executar(cpu_list, tarefas, relogio, quantum);
		}
		for (CPU *cpu : cpu_list)
		{
			Tarefa *t = cpu->getTarefa();

			if (t != nullptr)
			{
				t->executarUnidade();
				cout << "  [CPU " << cpu->getId() << "] Executando Tarefa " << t->getId()
					 << " | Restante: " << t->getTempoRestante() << "s" << endl;

				if (t->isConcluida())
				{
					cout << "  [TERMINOU] Tarefa " << t->getId() << " concluida na CPU " << cpu->getId() << endl;
					cpu->liberarCPU();
				}
			}
			else
			{
				cpu->unidadeDesligado();
			}
		}
	}
}
