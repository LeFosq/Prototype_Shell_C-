#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <map>
#include <vector>
#include <sstream>
#include <functional>
#include <dirent.h>
#include <sys/wait.h>

using namespace std;

vector<string> split(string, string);
void cd(string path);
void sair();
void ls();
void executar(const char* path, char* const argv[]);
void executarvp(const char* path, char* const argv[]);
void help();
void bemvindo();

int main()
{
    bemvindo();
    string prompt; // String que será o "input" do usuário
    char caminho[1024]; //Char em que o caminho é atribuído para ser mostrado no prompt.
    map<string, int> prompts; //Para para chavear as instruções (originalmente deveria ser só o CD, mas fiz o ls e um exit para aprendizado.
    prompts["cd"] = 1;
    prompts["exit"] = 2;
    prompts["ls"] = 3;
    prompts["help"] = 4;
    while(true)
    {
        getcwd(caminho,sizeof(caminho)); //Obtemos o diretório atual e atribuímos a variável "caminho"
        cout << caminho << " > "; //Mensagem que será exibida enquanto o BASH aguarda o input do usuário.
        getline(cin, prompt); //Aguardando input do usuário
        vector<string> vetorzinho;
        vetorzinho = split(prompt," ");

        if(prompts.count(vetorzinho[0]) > 0) //Separamos o input através dos espaços em um vetor chamado "vetorzinho", aqui verificamos se algo foi digitado
        {
            string arg = vetorzinho.size() > 1 ? vetorzinho[1]: "";
            char* argv[vetorzinho.size() +1];
            switch(prompts[vetorzinho[0]]) //Chaveamento que faz a equivalência da string com um número utilizado na case
            {
                case 1: //cd
                    cd(arg);
                    break;
                case 2: // exit
                    return 0;
                    break;
                case 3:
                    ls();
                    break;
                case 4:
                    help();
                    break;
                default:
                    cout << "Comando NÃO encontrado \n";
                    break;
            }


        }
        else if (vetorzinho[0].substr(0,2) == "./") //Caso "vetorzinho[0]" inicie com "./" sabemos que o usuário está tentando executar um arquivo no diretório atual, portanto encaminharemos para o execv
        {
            char* argv[vetorzinho.size() +1];
            for(size_t i = 0; i < vetorzinho.size(); i++)
            {
                argv[i] = const_cast<char*>(vetorzinho[i].c_str()); //Aqui estamos basicamente obtendo os argumentos que serão passados ao execv
            }
            argv[vetorzinho.size()] = nullptr;
            executar(vetorzinho[0].c_str(), argv); //Função que passa os argumentos ao execv
        }
        else //Caso não enquadre em nenhuma das situações acima passamos para um caso "geral" em que o programa vai utilizar o execvp para inicializar um programa no PATH.
        {
            char* argv[vetorzinho.size() + 1];
            for(size_t i = 0 ; i < vetorzinho.size(); i++)
            {
                argv[i] = const_cast<char*>(vetorzinho[i].c_str());
            }
            argv[vetorzinho.size()] = nullptr;
            executarvp(vetorzinho[0].c_str(), argv); //Tudo acontece igual ao IF acima, inclusive a função, sendo que a única mudança real é que aqui é utilizado o execvp e não o execv.
        }
    }
    return 0;
}

vector<string> split(string alvo, string delimitador) //Função para dividir o input do usuário de acordo com os " ".
{
    vector<string> separado;
    size_t x = alvo.find(delimitador);
    while(x != string::npos)
    {
        separado.push_back(alvo.substr(0,x));
        alvo = alvo.erase(0, x + delimitador.length());
        x = alvo.find(delimitador);
    }
    separado.push_back(alvo);
    return separado;
}

void cd(string path) //Função que utiliza o "chdir" para alterar o diretório atual.
{
    if (chdir(path.c_str()) != 0)
    {
        cout << "ERRO ao mudar de diretório";
    }
}

void sair()
{
    exit(0); //Função que fecha o bash.
}
void ls() //Função ls própria do meu bash
{
    vector<string> diretorios;
    int i = 0;
    int topo;
	DIR *dp;
	struct dirent* ep;

	dp = opendir("./");
	if(dp != nullptr)
	{
        while((ep = readdir(dp)) != nullptr)
        {
            diretorios.push_back(ep->d_name);
            i++;
        }
        closedir(dp);
        sort(diretorios.begin(), diretorios.end());
        topo = i;
        for(i = 0; i < topo ; i++)
        {
            cout<< diretorios[i] << "\t";
        }
        cout << "\n";
	}
	else
	{
        cout<< "Erro ao abrir diretório";
	}
}

void executar(const char* path, char* const argv[]) //Função para realizar o execv de inputs que começam com "./"
{
    pid_t pid = fork();
    if(pid == -1)
    {
        cout<< "FALHA ao criar processo filho \n";
    }
    if(pid == 0)
    {
        cout << "Processo filho criado com SUCESSO EXECV \n";
        execv(path,argv);
        cerr << "Falha ao executar \n";
        exit(1);
    }
    else
    {
        waitpid(pid, nullptr, 0);
    }
}

void executarvp(const char* path, char* const argv[]) //Função para realizar o excevp de inputs que não possuem "./" e nem se enquadram no switch na int main().
{
    pid_t pid = fork();
    if(pid == -1)
    {
        cout<< "FALHA ao criar processo filho EXECVP \n";
    }
    if(pid == 0)
    {
        cout << "Processo filho criado com SUCESSO EXECVP \n";
        execvp(path,argv);
        cerr << "Falha ao executar \n";
        exit(1);
    }
    else
    {
        waitpid(pid, nullptr, 0);
    }
}
void help()
{
    cout<< "O funcionamento desse shell se dá de forma bem simples. \nExistem três funções 'built in': \nO cd utilizado para alterar o diretório atual. \nO ls que lista os arquivos no diretório atual (não comporta nenhum argumento extra). \nO exit que serve para fechar o BASH \nO resto das funções é tratada via execvp e puxa diretamente do próprio linux.\n Ainda é possível rodar programas do diretório atual nativamente utilizando ./ (Não todos, já que é uma função implementada nativamente que não trabalhei tanto).\n ";
}
void bemvindo()
{
    cout<< "\033[1;32mOlá, bem vindo ao meu BASH \033[0m \n";
    cout<< "\033[1;31mAluno:\033[0m Luiz Fernando Almeida Pinheiro \n\033[1;31mMatrícula:\033[0m 221021151 \nTrabalho realizado para SOR 2024.1! \n";
    cout << "Insira o comando 'help' para descobrir mais sobre o BASH.\n";
}
