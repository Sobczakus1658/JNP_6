#include "worldcup.h"
#include "list"
#include <vector>
#include <stdbool.h>

class Player;
class Field;

enum state{
    inGame,
    waiting
    //, bankrupt //Myśle ze to wywalamy bo to nic nie daje nam, tylko przeszkadza
};

/* nie wiem czy to jest potrzebne
w sensie tak i tak potrzebujesz wiedziec ile czekasz
*/
class Player {
    private : 
        std::string const name;
        std::shared_ptr<Field> currentlyField;
        size_t wallet = 1000;
        state statement;
        int howManyWaiting;
        bool isBankrupt;

    public:
        Player(std::string const &name) : name(name), isBankrupt(false) {}
        bool takeMoney(size_t fee) {
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
        void setBankrupt(){
            isBankrupt = true;
        }
};

class Field {
    protected:
        std::string const name;
        Field(std::string const name) :
                name(name) {}
    public:
        virtual void passingAction(Player player){}
        virtual void landingAction(Player player){}
};

class Match : public virtual Field {
    protected:
        const double weight;
        const size_t fee;
        size_t prize;
    public:
        Match(std::string const name, double w, size_t f, size_t p):
            Field(name),
            weight(w),
            fee(f),
            prize(p) {}
        void passingAction(Player player) override {
            if(player.takeMoney(fee)){
                prize = prize + fee;
            }
        }
        void landingAction(Player player) override {
            player.addMoney(prize * weight);
        }
};

class SeasonBegin : public virtual Field {
    private:
        size_t passBonus = 50;
    public:
        SeasonBegin(std::string const name, size_t pB):
            Field(name),
            passBonus(pB) {}

        void passingAction(Player player) override {
            player.addMoney(passBonus);
        }

        void landingAction(Player player) override {
            // tu się nic nie dzieje chyba
        }
};

class Penalty : public virtual Field {
    private:
        size_t fee;
    public :
        Penalty(std::string const name, size_t f):
            Field(name),
            fee(f) {}

        void passingAction(Player player) override {
            //nic się nie dzieje
        }
        void landingAction(Player player) override {
            player.takeMoney(fee);
        }
};

class Goal : public virtual Field {
    private:
        size_t prize;
    public:
        Goal(std::string const name, size_t p):
            Field(name),
            prize(p) {}
        void passingAction(Player player) override {
            // nic się nie dzieje
        }
        void landingAction(Player player) override {
            player.addMoney(prize);
        }
};

class YellowCard : public virtual Field {
    private:
        uint64_t suspension;
    public:
        YellowCard(std::string const name, uint64_t susp):
            Field(name),
            suspension(susp) {}
        void passingAction(Player player) override {
            //nic się nie dzieje
        }
        void landingAction(Player player) override {
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
        void passingAction(Player player) override {
            //nic się nie dzieje
        }
        void landingAction(Player player) override {
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
        std::vector< std::shared_ptr<Die> > dices;
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
                // ?! Mieliśmy nie wywoływać jawnie destruktorów co tu się dzieje?
                //przecież wystarczy że stracisz wskaźnik na to skoro to jest sharedptr i to się samo usunie
                //i swoją drogą nie wiem co tu chcesz uzyskać
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