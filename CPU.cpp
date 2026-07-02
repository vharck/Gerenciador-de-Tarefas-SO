#pragma once
#include "raylib.h"
#include <vector>
#include <string>

namespace ProjetoSO
{
	class Tarefa
	{
		public:
			enum class TaskState {
				Idle,
				Ready,
				Executing,
				BlockedMutex,
				BlockedIO, 
				Finished
			};
			struct Acao {
				std::string tipo; // "ML", "MU" ou "IO"
				int id_mutex;     // Para ML e MU
				int instante;     // Instante relativo de disparo (em relação ao início da execução)
				int duracao;      // Duração apenas para IO
				bool concluida = false;
			};

			struct Event
			{
				TaskState state;
				unsigned int begin;
				unsigned int end;
				unsigned int priority = 0;
			};

		private:
			int id;
			Color color;
			int tempo_ingresso;
			int tempo_total;      // Tempo total que a tarefa precisa (lido do arquivo)
			int tempo_restante;   // Quanto ainda falta (essencial para SRTF)
			int prioridade;
			std::vector<Event> events;
			std::vector<Acao> acoes;
			int tempo_executado = 0; // Para saber em qual instante relativo estamos
			int tempo_io_restante = 0;

		public:
		
			// Construtor atualizado
			Tarefa(int id, Color cor, int ingresso, int t_execucao, int prioridade);
			// Getters
			Color getColor() const { return color; }
			int getTempoIngresso() const { return tempo_ingresso; }
			int getTempoTotal() const { return tempo_total; }
			int getTempoRestante() const { return tempo_restante; }
			int getPrioridade() const { return prioridade; }
			int getId() const { return id; }
			std::vector<Event> getEvents() const { return events; }
			void addAcao(Acao a) { acoes.push_back(a); }
			std::vector<Acao>& getAcoes() { return acoes; }
			int getTempoExecutado() const { return tempo_executado; }
			void incrementaTempoExecutado() { tempo_executado++; }
			int getTempoIORestante() const { return tempo_io_restante; }
			void setTempoIORestante(int t) { tempo_io_restante = t; }
			void decrementarTempoIORestante() { if (tempo_io_restante > 0) tempo_io_restante--; }
			

			// Métodos de Processamento
			void executarUnidade(); // Reduz o tempo restante
			bool isConcluida() const;
			void registrarEvento(TaskState state, unsigned int relogio);
			void resetEventos() { events.clear(); }
		
	};
}