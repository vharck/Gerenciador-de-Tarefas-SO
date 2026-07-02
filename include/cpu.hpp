#pragma once

#include "tarefa.hpp"

namespace ProjetoSO
{
	class Tarefa;

	class CPU
	{

	public:
	struct Event
	{
		unsigned int id;
		int clock;
		Tarefa *t;
	};
	private:
		int id;
		Tarefa *t;
		bool ocupado;
		int tempo_desligado;
		int tempo_exec;
		std::vector<Event> events;

	public:
		CPU(int id);

		// Getters
		int getId() const;
		Tarefa *getTarefa() const;
		bool isOcupado() const;
		int getTempo_desligado() const;
		int getTempo_exec() const;
		std::vector<Event> getEvents() const { return events; }

		// Setters e Métodos de Controle
		void setTarefa(Tarefa *nova_tarefa);
		void liberarCPU();
		void unidadeDesligado();
		void resetExec();
		void executando();
	};
}