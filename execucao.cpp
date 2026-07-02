#include "../include/escalonador.hpp"
#include <climits>
#include <cstdlib>

using namespace ProjetoSO;
using namespace std;

int Escalonador::buscarIndiceMelhor(const vector<Tarefa*>& fila,
									 const Comparador& melhorQue,
									 vector<Tarefa*>& empates) const
{
	empates.clear();
	if (fila.empty()) return -1;

	int idx = 0;

    empates.push_back(fila[0]);

	for (size_t i = 1; i < fila.size(); i++) {
		Tarefa* atual = fila[i];
		Tarefa* melhor = fila[idx];

		bool atualMelhor = melhorQue(atual, melhor);
		bool melhorMelhor = melhorQue(melhor, atual);

		if (!atualMelhor && !melhorMelhor) {
			if (atual->getTempoIngresso() == melhor->getTempoIngresso()) {
				empates.push_back(atual);
			} else if (atual->getTempoIngresso() < melhor->getTempoIngresso()) {
				empates.clear();
				idx = (int)i;
				empates.push_back(atual);
			}
		} else if (atualMelhor) {
			empates.clear();
			idx = (int)i;
			empates.push_back(atual);
		}
	}
	return idx;
}

CPU* Escalonador::buscarCPUAlvo(const vector<CPU*>& cpus, const Comparador& melhorQue, bool& encontrouNulo) const
{
	encontrouNulo = false;
	CPU* alvo = nullptr;
	for (CPU* cpu : cpus) {
		if (cpu->getTarefa() == nullptr) {
			encontrouNulo = true;
			return cpu;
		}
		// Substitui o alvo quando o alvo atual é "melhor" que o cpu candidato,
		// ou seja, quando o cpu candidato tem a tarefa PIOR entre os já vistos.
		if (alvo == nullptr || melhorQue(alvo->getTarefa(), cpu->getTarefa())) {
			alvo = cpu;
		}
	}
	return alvo;
}

Tarefa* Escalonador::sorteio(const vector<Tarefa*>& empates) const
{
	if (empates.empty()) return nullptr;
	return empates[rand() % empates.size()];
}

int Escalonador::indexOf(const vector<Tarefa*>& fila, Tarefa* t) const
{
	for (size_t i = 0; i < fila.size(); i++)
		if (fila[i] == t) return (int)i;
	return -1;
}

int Escalonador::tempoEsperando(Tarefa* t, unsigned int relogio) const
{
	const auto& events = t->getEvents();
	if (events.empty()) return 0;

	const auto& ultimo = events.back();
	if (ultimo.state == Tarefa::TaskState::Ready)
		return (int)relogio - (int)ultimo.begin;

	return 0;
}

void Escalonador::alocarSeVazio(vector<CPU*>& cpus, vector<Tarefa*>& fila, const Comparador& melhorQue, unsigned int relogio) const
{
	for (CPU* cpu : cpus) {
		if (cpu->getTarefa() != nullptr) continue;
		if (fila.empty()) break;

		vector<Tarefa*> empates;
		int idx = buscarIndiceMelhor(fila, melhorQue, empates);
		if (idx < 0) break;

		Tarefa* escolhida = fila[idx];
		if (empates.size() > 1) escolhida = sorteio(empates);

		idx = indexOf(fila, escolhida);
		cpu->setTarefa(escolhida);
		fila.erase(fila.begin() + idx);
		cpu->getTarefa()->registrarEvento(Tarefa::TaskState::Executing, relogio);
		cpu->resetExec();
	}
}

void Escalonador::alocarComPreempcao(vector<CPU*>& cpusElegiveis,
									  vector<CPU*>& cpusTodos,
									  vector<Tarefa*>& fila,
									  const Comparador& melhorQue,
									  const Comparador& piorQue,
									  const Comparador& deveSubstituir,
									  unsigned int relogio,
									  bool resetExecAoTrocar) const
{
	for (size_t n = 0; n < cpusElegiveis.size(); n++) {
		if (fila.empty()) break;

		vector<Tarefa*> empates;
		int idx = buscarIndiceMelhor(fila, melhorQue, empates);
		if (idx < 0) break;
		Tarefa* melhorDaFila = fila[idx];

		bool encontrouNulo = false;
		CPU* cpuAlvo = buscarCPUAlvo(cpusTodos, piorQue, encontrouNulo);
		if (cpuAlvo == nullptr) break;

		if (encontrouNulo) {
			if (empates.size() > 1) melhorDaFila = sorteio(empates);
			idx = indexOf(fila, melhorDaFila);

			cpuAlvo->setTarefa(melhorDaFila);
			fila.erase(fila.begin() + idx);
			cpuAlvo->getTarefa()->registrarEvento(Tarefa::TaskState::Executing, relogio);
			if (resetExecAoTrocar) cpuAlvo->resetExec();
		}
		else if (deveSubstituir(melhorDaFila, cpuAlvo->getTarefa())) {
			if (empates.size() > 1) melhorDaFila = sorteio(empates);
			idx = indexOf(fila, melhorDaFila);

			Tarefa* saindo = cpuAlvo->getTarefa();
			cpuAlvo->setTarefa(melhorDaFila);
			fila.erase(fila.begin() + idx);
			cpuAlvo->getTarefa()->registrarEvento(Tarefa::TaskState::Executing, relogio);
			saindo->registrarEvento(saindo->isConcluida() ? Tarefa::TaskState::Finished : Tarefa::TaskState::Ready, relogio);
			fila.push_back(saindo);
			if (resetExecAoTrocar) cpuAlvo->resetExec();
		}
		else {
			break;
		}
	}
}

// ---------------- FCFS ----------------
void FCFSEscalonador::executar(vector<CPU*>& cpus, vector<Tarefa*>& fila, unsigned int relogio, unsigned int)
{
	Comparador porIngresso = [](Tarefa*, Tarefa*) { return false; };
	alocarSeVazio(cpus, fila, porIngresso, relogio);
}

// ---------------- SJF ----------------
void SJFEscalonador::executar(vector<CPU*>& cpus, vector<Tarefa*>& fila, unsigned int relogio, unsigned int)
{
	Comparador menorTotal = [](Tarefa* a, Tarefa* b) {
		return a->getTempoTotal() < b->getTempoTotal();
	};
	alocarSeVazio(cpus, fila, menorTotal, relogio);
}

// ---------------- SRTF ----------------
void SRTFEscalonador::executar(vector<CPU*>& cpus, vector<Tarefa*>& fila, unsigned int relogio, unsigned int quantum)
{
	vector<CPU*> elegiveis;
	for (CPU* cpu : cpus)
		if (cpu->getTarefa() == nullptr || cpu->getTempo_exec() % quantum == 0)
			elegiveis.push_back(cpu);

	Comparador menorRestante = [](Tarefa* a, Tarefa* b) {
		return a->getTempoRestante() < b->getTempoRestante();
	};
	alocarComPreempcao(elegiveis, cpus, fila, menorRestante, menorRestante, menorRestante, relogio, true);
}

// ---------------- PRIOP ----------------
void PRIOPEscalonador::executar(vector<CPU*>& cpus, vector<Tarefa*>& fila, unsigned int relogio, unsigned int)
{
	Comparador maiorPrioridade = [](Tarefa* a, Tarefa* b) {
		return a->getPrioridade() > b->getPrioridade();
	};
	alocarComPreempcao(cpus, cpus, fila, maiorPrioridade, maiorPrioridade, maiorPrioridade, relogio, false);
}

// ---------------- PRIOP com envelhecimento ----------------
int PRIOPEnvelhecimentoEscalonador::prioridadeEfetiva(Tarefa* t, unsigned int relogio) const
{
	int bonus = tempoEsperando(t, relogio) / (int)intervaloEnvelhecimento;
	return t->getPrioridade() + bonus;
}

void PRIOPEnvelhecimentoEscalonador::executar(vector<CPU*>& cpus, vector<Tarefa*>& fila,
											   unsigned int relogio, unsigned int)
{
	Comparador maiorPrioridadeEfetiva = [this, relogio](Tarefa* a, Tarefa* b) {
		return prioridadeEfetiva(a, relogio) > prioridadeEfetiva(b, relogio);
	};
	alocarComPreempcao(cpus, cpus, fila,
						maiorPrioridadeEfetiva, maiorPrioridadeEfetiva, maiorPrioridadeEfetiva,
						relogio, false);
}

// ---------------- PRIOd ----------------
void PRIOdEscalonador::executar(vector<CPU*>& cpus, vector<Tarefa*>& fila, unsigned int relogio, unsigned int quantum)
{
	vector<CPU*> elegiveis;
	for (CPU* cpu : cpus)
		if (cpu->getTarefa() == nullptr || cpu->getTempo_exec() % quantum == 0)
			elegiveis.push_back(cpu);

	Comparador maiorPrioridade = [](Tarefa* a, Tarefa* b) {
		return a->getPrioridade() > b->getPrioridade();
	};
	alocarComPreempcao(elegiveis, cpus, fila, maiorPrioridade, maiorPrioridade, maiorPrioridade, relogio, true);
}

// ---------------- Round-Robin ----------------
void RoundRobinEscalonador::executar(vector<CPU*>& cpus, vector<Tarefa*>& fila, unsigned int relogio, unsigned int quantum)
{
	for (CPU* cpu : cpus) {
		Tarefa* t = cpu->getTarefa();

		if (t == nullptr) {
			if (fila.empty()) continue;
			Tarefa* proxima = fila.front();
			fila.erase(fila.begin());
			cpu->setTarefa(proxima);
			proxima->registrarEvento(Tarefa::TaskState::Executing, relogio);
			cpu->resetExec();
			continue;
		}

		if (cpu->getTempo_exec() > 0 && cpu->getTempo_exec() % quantum == 0) {
			if (fila.empty()) continue;

			Tarefa* proxima = fila.front();
			fila.erase(fila.begin());

			cpu->setTarefa(proxima);
			proxima->registrarEvento(Tarefa::TaskState::Executing, relogio);
			t->registrarEvento(t->isConcluida() ? Tarefa::TaskState::Finished : Tarefa::TaskState::Ready, relogio);
			fila.push_back(t);
			cpu->resetExec();
		}
	}
}