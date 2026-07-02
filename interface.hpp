#pragma once
#include <vector>
#include <string>
#include <memory>
#include "cpu.hpp"
#include "escalonador.hpp"
#include <map>
#include <queue>

namespace ProjetoSO 
{
	class Execucao 
	{
		private:
			std::string type;
			std::unique_ptr<Escalonador> escalonador;
			std::vector<CPU*> cpu_list;
			std::vector<Tarefa*> tarefas;
			std::vector<Tarefa*> todas_tarefas;
			std::vector<Tarefa*> finalizadas;
			unsigned int quantum;
			unsigned int alpha;
			int relogio;
			std::map<int, Tarefa*> mutex_owner;                      // id_mutex -> Tarefa* que o possui
			std::map<int, std::queue<Tarefa*>> mutex_wait_queue;     // Fila de espera do mutex
			std::vector<Tarefa*> lista_io;                           // Tarefas atualmente em E/S
			std::map<int, Tarefa*> mutex_donos;              // id_mutex -> Tarefa que possui o lock
			std::map<int, std::queue<Tarefa*>> mutex_filas;  // id_mutex -> fila de tarefas bloqueadas esperando o recurso
			std::vector<Tarefa*> tarefas_em_io;

			void entra_tarefa();
			void quantum_tempo();
			std::unique_ptr<Escalonador> criarEscalonador(const std::string& type, unsigned int quantum);

		public:

			Execucao() = default;
			Execucao(const Execucao&) = delete;
			void operator=(const Execucao&) = delete;

			static Execucao* getInstance();
			void init(std::string type, std::vector<Tarefa*> todas_tarefas, unsigned int quantum, unsigned int cpuCount, unsigned alpha);
			void finalizar_tarefa(Tarefa* t);
			void mandar_fila_prontos(Tarefa* t);
			void mandar_cpu(Tarefa* t);
			bool update();
			void unidade_tempo();
			void automatico();
			void processarAcoes(CPU* cpu, Tarefa* t);
			void atualizarIO();

			std::vector<CPU*> getCpuList() const {return cpu_list;}
			std::vector<Tarefa*> getTarefas() const {return tarefas;}
			std::vector<Tarefa*> getFinalizadas() const {return finalizadas;}
			int getRelogio() const {return relogio;}
			bool getCPUVazio();
	};
}