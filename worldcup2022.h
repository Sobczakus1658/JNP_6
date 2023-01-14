#ifndef WORLDCUP2022_H
#define WORLDCUP2022_H

#include "worldcup.h"
#include "list"
#include <vector>

class TooManyPlayersException : std::exception {};
class TooManyDiceException : std::exception {};
class TooFewPlayersException : std::exception {};
class TooFewDiceException : std::exception {};

class Player {
    private :
        std::string const name;
        size_t currentlyField;
        unsigned int wallet = 1000;
        unsigned int state;
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

        void addMoney(double prize) {
            wallet = wallet + prize;
        }

        size_t getCurrField() const {
            return currentlyField;
        }

        bool skipsTurn() const {
            return state > 0;
        }

        void waitOneTurn() {
            assert(state > 0);
            state--;
        }

        bool getIsBankrupt() const {
            return isBankrupt;
        }

        void setFine(int fine) {
            state = fine;
        }

        std::string getName() const {
            return name;
        }

        void setBankrupt() {
            isBankrupt = true;
        }

        void moveOneField(size_t boardSize) {
            currentlyField = (currentlyField + 1) % boardSize;
        }

        void writeScore(const std::shared_ptr<ScoreBoard>& scoreBoard, std::string const &fieldName) {
            if (isBankrupt) {
                scoreBoard->onTurn(name, "*** bankrut ***", fieldName, 0);
                return;
            }
            if (state > 0) {
                //czeka ture
                scoreBoard->onTurn(name, "*** czekanie: " + std::to_string(state) + " ***", fieldName, wallet);
                return;
            }
            scoreBoard->onTurn(name, "w grze", fieldName, wallet);
        }
};

class Field {
    protected:
        std::string const name;
        Field(std::string const &name) :
                name(name) {}
    public:
        virtual void passingAction(Player * player){}

        virtual void landingAction(Player * player){}

        std::string getName() const {
            return name;
        }
};

class Match : public virtual Field {
    protected:
        const double weight;
        const size_t fee;
        size_t prize;
    public:
        Match(std::string const &name, double w, size_t f):
                Field(name),
                weight(w),
                fee(f),
                prize(0) {}

        void passingAction(Player * player) override {
            if(player->takeMoney(fee)){
                prize = prize + fee;
            }
        }

        void landingAction(Player * player) override {
            player->addMoney(prize * weight);
        }
};

class SeasonBegin : public virtual Field {
    private:
        size_t passBonus;
    public:
        SeasonBegin(std::string const &name, size_t pB):
                Field(name),
                passBonus(pB) {}

        void passingAction(Player * player) override {
            player->addMoney(passBonus);
        }

        void landingAction(Player * player) override {
            // tu się nic nie dzieje chyba
        }
};

class Penalty : public virtual Field {
    private:
        size_t fee;
    public:
        Penalty(std::string const &name, size_t f):
                Field(name),
                fee(f) {}

        void passingAction(Player * player) override {
            //nic się nie dzieje
        }

        void landingAction(Player * player) override {
            player->takeMoney(fee);
        }
};

class Goal : public virtual Field {
    private:
        size_t prize;
    public:
        Goal(std::string const &name, size_t p):
                Field(name),
                prize(p) {}

        void passingAction(Player * player) override {
            // nic się nie dzieje
        }

        void landingAction(Player * player) override {
            player->addMoney(prize);
        }
};

class YellowCard : public virtual Field {
    private:
        uint64_t suspension;
    public:
        YellowCard(std::string const &name, uint64_t susp):
                Field(name),
                suspension(susp) {}

        void passingAction(Player * player) override {
            // nic się nie dzieje
        }

        void landingAction(Player * player) override {
            player->setFine(suspension);
        }
};

class Bookmaker : public virtual Field {
    private:
        int player_counter = 0;
        size_t fee;
        size_t prize;
        int playerCycle = 3;
    public:

        Bookmaker(std::string const & name, size_t f, size_t p):
            Field(name),
            fee(f),
            prize(p) {}

        void passingAction(Player * player) override {
            //nic się nie dzieje
        }

        void landingAction(Player * player) override {
            if(player_counter % playerCycle == 0){
                player->addMoney(prize);
            } else {
                player->takeMoney(fee);
            }
            ++player_counter;
        }
};

class RestDay : public virtual Field {
    public:
        RestDay(std::string const & name):
            Field(name) {}
        void landingAction(Player *player) override {}
        void passingAction(Player *player) override {}
};

class Board {
    private:
        std::vector< std::shared_ptr<Field> > fields;
    public:

        Board():
        fields() {
            fields.push_back(std::make_shared<SeasonBegin>("Początek sezonu", 50));
            fields.push_back(std::make_shared<Match>("Mecz z San Marino", 1, 160));
            fields.push_back(std::make_shared<RestDay>("Dzień wolny od treningu"));
            fields.push_back(std::make_shared<Match>("Mecz z Liechtensteinem", 1, 220));
            fields.push_back(std::make_shared<YellowCard>("Żółta kartka", 3));
            fields.push_back(std::make_shared<Match>("Mecz z Meksykiem", 2.5, 300));
            fields.push_back(std::make_shared<Match>("Mecz z Arabią Saudyjską", 2.5, 280));
            fields.push_back(std::make_shared<Bookmaker>("Bukmacher", 100, 100));
            fields.push_back(std::make_shared<Match>("Mecz z Argentyną", 2.5, 250));
            fields.push_back(std::make_shared<Goal>("Gol", 120));
            fields.push_back(std::make_shared<Match>("Mecz z Francją", 5, 400));
            fields.push_back(std::make_shared<Penalty>("Rzut karny", 180));

        }

        size_t size() const {
            return fields.size();
        }

        Field * operator[] (size_t i) const { //nie jestem jakoś bardzo pewny tego consta jak coś ~~FS
            return fields[i].get();
        }
};

class WorldCup2022 : public WorldCup {
    private :
        std::vector<std::shared_ptr<Player>> players;
        std::shared_ptr<ScoreBoard> scoreboard;
        std::vector<std::shared_ptr<Die>> dices;
        int bankruptNumber = 0;
        Board board;
    public:
        WorldCup2022():
        players(),
        scoreboard(),
        dices(),
        board() {

            // IMO ten kod nie dość że miał być w destruktrorze, to jeszcze jest obrzydliwy i nie działa:
//            if(scoreboard.unique()){
//                // gameScoreboard.~Scoreboard();
//                scoreboard.reset();
//            }
//            for (auto &dice : dices){
//                if(dice.unique()) {
//                    //gameDie.~die();
//                    dice.reset();
//                }
//            }
        }

        // Jeżeli argumentem jest pusty wskaźnik, to nie wykonuje żadnej operacji
        // (ale nie ma błędu).
        void addDie(std::shared_ptr<Die> die) {
            if (die == nullptr) {
                return;
            }
            dices.push_back(die);
        }

        // Dodaje nowego gracza o podanej nazwie.
        void addPlayer(std::string const &name){
            std::shared_ptr<Player> playerPtr = std::make_shared<Player>(name);
            players.push_back(playerPtr);
        }

        // Konfiguruje tablicę wyników. Domyślnie jest skonfigurowana tablica
        // wyników, która nic nie robi.
        void setScoreBoard(std::shared_ptr<ScoreBoard> sc) {
            scoreboard = sc; //Sprawdzone, ma być tak jednak ~~FS
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
        void play(unsigned int rounds) {
            if (players.size() < 2) {
                throw TooFewPlayersException();
            } else if (players.size() > 11) {
                throw TooManyPlayersException();
            }

            if (dices.size() < 2) {
                throw TooFewDiceException();
            } else if (dices.size() > 2) {
                throw TooManyDiceException();
            }

            //poprawnie wszystko można grać
            for (unsigned int roundNo = 0; roundNo < rounds; roundNo++) {
                scoreboard->onRound(roundNo);
                for (std::shared_ptr<Player> player : players) {
                    //czeka
                    if (!player->getIsBankrupt() && !player->skipsTurn()) {
                        //gra
                        unsigned int roll = 0;
                        for (const std::shared_ptr<Die>& die: dices) {
                            roll += die->roll();
                        }
                        for (size_t i = 1; i <= roll - 1; i++) {
                            player->moveOneField(board.size());
                            board[player->getCurrField()]->passingAction(player.get());
                            if (player->getIsBankrupt()) {
                                ++bankruptNumber;
                                player->writeScore(scoreboard, board[player->getCurrField()]->getName());
                                break;
                            }
                        }
                        if (!player->getIsBankrupt()) {
                            player->moveOneField(board.size());
                            board[player->getCurrField()]->landingAction(player.get());
                        }
                    }

                    if (!player->getIsBankrupt()) {
                        player->writeScore(scoreboard, board[player->getCurrField()]->getName());
                    }

                    if (player->skipsTurn()) {
                        player->waitOneTurn();
                    }

                    if (bankruptNumber == players.size() - 1) {
                        for (std::shared_ptr<Player> p : players) {
                            if (!p->getIsBankrupt())
                                scoreboard->onWin(p->getName());
                        }
                        return;
                    }
                }
            }
        }
};

#endif