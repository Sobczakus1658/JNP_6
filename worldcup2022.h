#include "worldcup.h"
#include "list"
#include <vector>
#include <stdbool.h>

enum state{
    inGame,
    waiting,
    bankrupt
};

/* nie wiem czy to jest potrzebne
w sensie tak i tak potrzebujesz wiedziec ile czekasz
*/
class Player {
    private : 
        std::string const name;
        Field currentlyField;
        size_t wallet = 1000;
        state statement;
        int howManyWaiting;
        void setBankrupt(){

        }

    public:
        Player( std::string const &name) : name(name) {}
        bool takeMoney(size_t fee){
            if(wallet >= fee){
                wallet = wallet - fee;
                return true;
            } else {
                setBankrupt();
                return false;
            }
        }
        void addMoney(size_t prize){
            wallet = wallet + prize;
        }
        void setFine(int fine){
            howManyWaiting = fine;
        }
};

class Field { 
    protected: 
        std::string const name;
    public:
        virtual void passingAction(Player player){}
        virtual void landingAction(Player player){}
};

class Match : public virtual Field {
    protected:
        double weighted;
        size_t fee;
        size_t prize;
    public:
        void passingAction(Player player){
            if(player.takeMoney(fee)){
                prize = prize + fee;
            }
        }
        void landingAction(Player player){
            player.addMoney(prize * weighted);
        }
};

class SeasonBegin : public virtual Field {
    private:
        size_t passBonus = 50;
    public: 
        void passingAction(Player player){
            player.addMoney(passBonus);
        }
        void landingAction(Player player){
            // tu się nic nie dzieje chyba
        }
};

class Penalty : public virtual Field {
    private:
        size_t fee;
    public :
        void passingAction(Player player){
            //nic się nie dzieje
        }
        void landingAction(Player player){
            player.takeMoney(fee);
        }
};

class Goal : public virtual Field {
    private:
        size_t prize;
    public:    
        void passingAction(Player player){
            // nic się nie dzieje
        }
        void landingAction(Player player){  
            player.addMoney(prize);
        }
};

class YellowCard : public virtual Field {
    private:
        int suspension;
    void passingAction(Player player){
        //nic się nie dzieje
    }
    void landingAction(Player player){
        player.setFine(suspension);
    }
};

class Bookmaker : public virtual Field {
    private:
        int player_counter;
        size_t fee;
        size_t prize;
        int playerCycle = 3; //nie wiem jak to nazwac =
    public:
        void passingAction(Player player){
            //nic się nie dzieje
        }
        void landingAction(Player player){
            if(player_counter % playerCycle == 0){
                player.addMoney(prize);
            }
            else{
                player.takeMoney(fee);
            }
        }
};

class Board {
    std::list<Field> field;
};

class WorldCup2022 : public WorldCup{
    private :
        std::list<Player> players; //chcesz cykliczną listę, nie wiem nie do końca o co chodzi
        std::shared_ptr<ScoreBoard> scoreboard;
        std::vector<std::shared_ptr<Die> > dices;
        class Board;
        
public:
 
    //trzeba napisać swój deskturktor, żeby odpiąć die i ScoreBoard

    // jeżeli jestem jedynym właścicielm share_pointera to również usuwam
    // nie umiem poprawnie wywołać destruktor, potem poprawię
    WorldCup2022(){
        if(scoreboard.unique()){
           // gameScoreboard.~Scoreboard();
        }
        for( int i = 0; i < dices.size(); i++){
            if(dices[i].unique()){
                //gameDie.~die();
            }
        }
    }

    // Jeżeli argumentem jest pusty wskaźnik, to nie wykonuje żadnej operacji
    // (ale nie ma błędu).
    void addDie(std::shared_ptr<Die> die){
        if(dices.size() >= 2){
//            throw std::exception("ToManyDiceException");
        }
        dices.push_back(die);
    }

    // Dodaje nowego gracza o podanej nazwie.
    void addPlayer(std::string const &name){
        if(players.size() > 11){
//            throw std::exception("ToManyPlayerException");
        }
        Player player(name);
        players.push_back(player);
    }

    // Konfiguruje tablicę wyników. Domyślnie jest skonfigurowana tablica
    // wyników, która nic nie robi.

    //TODO
    /*
        każda gra będzie miała jeden scoreboard ?, więc mogę go chyba przepisać
        nie będzie dwóch gier które maja tego samego scoreboarda. 
        Jeżeli jednak tak będzie to nie wiem w sumie co z tym zrobić, jakiego typu chcemy trzymać scoreboard ?
    */
    void setScoreBoard(std::shared_ptr<ScoreBoard> scoreboard){
        scoreboard = scoreboard;
    }

    // Przeprowadza rozgrywkę co najwyżej podanej liczby rund (rozgrywka może
    // skończyć się wcześniej).
    // Jedna runda obejmuje po jednym ruchu każdego gracza.
    // Gracze ruszają się w kolejności, w której zostali dodani.
    // Na początku każdej rundy przekazywana jest informacja do tablicy wyników
    // o początku rundy (onRound), a na zakończenie tury gracza informacje
    // podsumowujące dla każdego gracza (onTurn).
    // Rzuca TooManyDiceException, jeśli jest zbyt dużo kostek.
    // Rzuca TooFewDiceException, jeśli nie ma wystarczającej liczby kostek.
    // Rzuca TooManyPlayersException, jeśli jest zbyt dużo graczy.
    // Rzuca TooFewPlayersException, jeśli liczba graczy nie pozwala na
    // rozpoczęcie gry.
    // Wyjątki powinny dziedziczyć po std::exception.
    void play(unsigned int rounds){
        if(players.size() < 2) {
//            throw std::exception("TooFewDiceException");
        }
        if(dices.size() < 2) {
//            throw std::exception("TooFewDiceException");
        }
        //poprawnie wszytsko można grać

    }
};