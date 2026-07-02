#pragma once
#include <vector>
#include <functional>
#include <string>
#include "cpu.hpp"
#include "tarefa.hpp"

namespace ProjetoSO
{
	class Escalonador
	{
	protected:
		using Comparador = std::function<bool(Tarefa*, Tarefa*)>;

		int buscarIndiceMelhor(const std::vector<Tarefa*>& fila,
								const Comparador& melhorQue,
								std::vector<Tarefa*>& empates) const;

		CPU* buscarCPUAlvo(const std::vector<CPU*>& cpus,
							const Comparador& piorQue,
							bool& encontrouNulo) const;

		Tarefa* sorteio(const std::vector<Tarefa*>& empates) const;

		int indexOf(const std::vector<Tarefa*>& fila, Tarefa* t) const;

		void alocarSeVazio(std::vector<CPU*>& cpus,
							std::vector<Tarefa*>& fila,
							const Comparador& melhorQue,
							unsigned int relogio) const;

		void alocarComPreempcao(std::vector<CPU*>& cpusElegiveis,
								 std::vector<CPU*>& cpusTodos,
								 std::vector<Tarefa*>& fila,
								 const Comparador& melhorQue,
								 const Comparador& piorQue,
								 const Comparador& deveSubstituir,
								 unsigned int relogio,
								 bool resetExecAoTrocar) const;

		int tempoEsperando(Tarefa* t, unsigned int relogio) const;

	public:
		virtual ~Escalonador() = default;

		virtual void executar(std::vector<CPU*>& cpuList,
							   std::vector<Tarefa*>& fila,
							   unsigned int relogio,
							   unsigned int quantum) = 0;

		virtual std::string nome() const = 0;
	};

	class FCFSEscalonador : public Escalonador
	{
	public:
		void executar(std::vector<CPU*>&, std::vector<Tarefa*>&, unsigned int, unsigned int) override;
		std::string nome() const override { return "FCFS"; }
	};

	class SJFEscalonador : public Escalonador
	{
	public:
		void executar(std::vector<CPU*>&, std::vector<Tarefa*>&, unsigned int, unsigned int) override;
		std::string nome() const override { return "SJF"; }
	};

	class SRTFEscalonador : public Escalonador
	{
	public:
		void executar(std::vector<CPU*>&, std::vector<Tarefa*>&, unsigned int, unsigned int) override;
		std::string nome() const override { return "SRTF"; }
	};

	class PRIOPEscalonador : public Escalonador
	{
	public:
		void executar(std::vector<CPU*>&, std::vector<Tarefa*>&, unsigned int, unsigned int) override;
		std::string nome() const override { return "PRIOP"; }
	};

	class PRIOPEnvelhecimentoEscalonador : public Escalonador
	{
	private:
		unsigned int intervaloEnvelhecimento;
		int prioridadeEfetiva(Tarefa* t, unsigned int relogio) const;

	public:
		explicit PRIOPEnvelhecimentoEscalonador(unsigned int intervalo = 5)
			: intervaloEnvelhecimento(intervalo > 0 ? intervalo : 1) {}

		void executar(std::vector<CPU*>&, std::vector<Tarefa*>&, unsigned int, unsigned int) override;
		std::string nome() const override { return "PRIOP-ENV"; }
	};

	class PRIOdEscalonador : public Escalonador
	{
	public:
		void executar(std::vector<CPU*>&, std::vector<Tarefa*>&, unsigned int, unsigned int) override;
		std::string nome() const override { return "PRIOd"; }
	};

	class RoundRobinEscalonador : public Escalonador
	{
	public:
		void executar(std::vector<CPU*>&, std::vector<Tarefa*>&, unsigned int, unsigned int) override;
		std::string nome() const override { return "RR"; }
	};
}