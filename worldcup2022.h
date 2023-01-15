#ifndef WORLDCUP2022_H
#define WORLDCUP2022_H

#include "worldcup.h"
#include <vector>

class TooManyPlayersException : public std::exception {
};

class TooManyDiceException : public std::exception {
};

class TooFewPlayersException : public std::exception {
};

class TooFewDiceException : public std::exception {
};

class Player {
private :
    std::string const name;
    size_t currentlyField = 0;
    unsigned int wallet = 1000;
    unsigned int state = 0; // 0 gdy jest w grze, n > 0 gdy musi poczekac jeszcze n tur
    bool isBankrupt;
public:
    explicit Player(std::string const &name) : name(name), isBankrupt(false) {}

    size_t takeMoney(size_t fee) {
        if (wallet >= fee) {
            wallet = wallet - fee;
            return fee;
        } else {
            setBankrupt();
            return wallet;
        }
    }

    void addMoney(double prize) {
        wallet = wallet + (unsigned int) prize;
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

    void setFine(size_t fine) {
        state = fine;
    }

    std::string getName() const {
        return name;
    }

    unsigned int getWallet() const {
        return wallet;
    }

    void setBankrupt() {
        isBankrupt = true;
    }

    void moveOneField(size_t boardSize) {
        currentlyField = (currentlyField + 1) % boardSize;
    }

    void writeScore(const std::shared_ptr <ScoreBoard> &scoreBoard, std::string const &fieldName) {
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

    explicit Field(std::string const &name) :
            name(name) {}

public:
    virtual void passingAction([[maybe_unused]] Player *player) {}

    virtual void landingAction([[maybe_unused]] Player *player) {}

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
    Match(std::string const &name, double w, size_t f) :
            Field(name),
            weight(w),
            fee(f),
            prize(0) {}

    void passingAction(Player *player) override {
        prize = prize + player->takeMoney(fee);
    }

    void landingAction(Player *player) override {
        player->addMoney((double) prize * weight);
        prize = 0;
    }
};

class SeasonBegin : public virtual Field {
private:
    size_t passBonus;
public:
    SeasonBegin(std::string const &name, size_t pB) :
            Field(name),
            passBonus(pB) {}

    void passingAction(Player *player) override {
        player->addMoney((double) passBonus);
    }

    void landingAction(Player *player) override {
        player->addMoney((double) passBonus);
    }
};

class Penalty : public virtual Field {
private:
    size_t fee;
public:
    Penalty(std::string const &name, size_t f) :
            Field(name),
            fee(f) {}

    void landingAction(Player *player) override {
        player->takeMoney(fee);
    }
};

class Goal : public virtual Field {
private:
    size_t prize;
public:
    Goal(std::string const &name, size_t p) :
            Field(name),
            prize(p) {}

    void landingAction(Player *player) override {
        player->addMoney((double) prize);
    }
};

class YellowCard : public virtual Field {
private:
    uint64_t suspension;
public:
    YellowCard(std::string const &name, uint64_t susp) :
            Field(name),
            suspension(susp) {}

    void landingAction(Player *player) override {
        player->setFine(suspension);
    }
};

class Bookmaker : public virtual Field {
private:
    int playerCounter = 0;
    size_t fee;
    size_t prize;
    int playerCycle = 3;
public:

    Bookmaker(std::string const &name, size_t f, size_t p) :
            Field(name),
            fee(f),
            prize(p) {}

    void landingAction(Player *player) override {
        if (playerCounter % playerCycle == 0) {
            player->addMoney((double) prize);
        } else {
            player->takeMoney(fee);
        }
        playerCounter = (playerCounter + 1) % playerCycle;
    }
};

class RestDay : public virtual Field {
public:
    explicit RestDay(std::string const &name) :
            Field(name) {}
};

class Board {
private:
    std::vector <std::shared_ptr<Field>> fields;
public:

    Board() :
            fields() {
        fields.push_back(std::make_shared<SeasonBegin>("Początek sezonu", 50));
        fields.push_back(std::make_shared<Match>("Mecz z San Marino", 1, 160));
        fields.push_back(std::make_shared<RestDay>("Dzień wolny od treningu"));
        fields.push_back(std::make_shared<Match>("Mecz z Lichtensteinem", 1, 220));
        fields.push_back(std::make_shared<YellowCard>("Żółta kartka", 3));
        fields.push_back(std::make_shared<Match>("Mecz z Meksykiem", 2.5, 300));
        fields.push_back(std::make_shared<Match>("Mecz z Arabią Saudyjską", 2.5, 280));
        fields.push_back(std::make_shared<Bookmaker>("Bukmacher", 100, 100));
        fields.push_back(std::make_shared<Match>("Mecz z Argentyną", 2.5, 250));
        fields.push_back(std::make_shared<Goal>("Gol", 120));
        fields.push_back(std::make_shared<Match>("Mecz z Francją", 4, 400));
        fields.push_back(std::make_shared<Penalty>("Rzut karny", 180));

    }

    size_t size() const {
        return fields.size();
    }

    Field *operator[](size_t i) const {
        return fields[i].get();
    }
};

class Dice {
private:
    std::vector <std::shared_ptr<Die>> dice;
public:
    Dice() : dice() {}

    unsigned short roll() {
        unsigned short ret = 0;
        for (std::shared_ptr <Die> die: dice) {
            ret += die->roll();
        }
        return ret;
    }

    size_t getNumberOfDices() const {
        return dice.size();
    }

    // Jeżeli argumentem jest pusty wskaźnik, to nie wykonuje żadnej operacji
    // (ale nie ma błędu).
    void addDie(std::shared_ptr <Die> die) {
        if (!die) {
            return;
        }
        dice.push_back(die);
    }
};

class WorldCup2022 : public WorldCup {
private :
    std::vector <std::shared_ptr<Player>> players;
    std::shared_ptr <ScoreBoard> scoreboard;
    std::shared_ptr <Dice> dice;
    size_t bankruptNumber = 0;
    Board board;
public:
    WorldCup2022() {
        dice = std::make_shared<Dice>(Dice());
    }

    // Dodaje nowego gracza o podanej nazwie.
    void addPlayer(std::string const &name) {
        std::shared_ptr <Player> playerPtr = std::make_shared<Player>(name);
        players.push_back(playerPtr);
    }

    void addDie(std::shared_ptr <Die> die) {
        dice->addDie(die);
    }

    // Konfiguruje tablicę wyników. Domyślnie jest skonfigurowana tablica
    // wyników, która nic nie robi.
    void setScoreBoard(std::shared_ptr <ScoreBoard> sc) {
        scoreboard = sc;
    }

    // Zakładamy że po zakończeniu rozgrywki będzie tylko jeden zwycięzca,
    // nie będzie remisów.
    void findWinner() {
        Player *winner = players[0].get();
        for (const std::shared_ptr <Player> &player: players) {
            if (player->getWallet() > winner->getWallet()) {
                winner = player.get();
            }
        }
        scoreboard->onWin(winner->getName());
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
        if (dice->getNumberOfDices() < 2) {
            throw TooFewDiceException();
        } else if (dice->getNumberOfDices() > 2) {
            throw TooManyDiceException();
        }
        if (players.size() < 2) {
            throw TooFewPlayersException();
        } else if (players.size() > 11) {
            throw TooManyPlayersException();
        }

        // Zakładamy, że osoba, która zbankrutowała nadal jest w grze,
        // więc po zakończeniu każdej tury wypisze się, że jest bankrutem.
        for (unsigned int roundNo = 0; roundNo < rounds; roundNo++) {
            scoreboard->onRound(roundNo);
            for (const std::shared_ptr <Player> &player: players) {
                if (!player->getIsBankrupt() && !player->skipsTurn()) {
                    //gra
                    unsigned int roll = dice->roll();
                    for (size_t i = 1; i + 1 <= roll; i++) {
                        player->moveOneField(board.size());
                        board[player->getCurrField()]->passingAction(player.get());
                        if (player->getIsBankrupt()) {
                            ++bankruptNumber;
                            for (size_t j = i; j <= roll - 1; j++) {
                                player->moveOneField(board.size());
                            }
                            break;
                        }
                    }
                    if (!player->getIsBankrupt()) {
                        if (roll != 0) {
                            player->moveOneField(board.size());
                        }
                        board[player->getCurrField()]->landingAction(player.get());
                    }
                }
                player->writeScore(scoreboard, board[player->getCurrField()]->getName());

                if (player->skipsTurn()) {
                    player->waitOneTurn();
                }

                // W sytuacji kiedy tylko jeden gracz nie jest bankrutem,
                // gra się kończy.
                if (bankruptNumber == players.size() - 1) {
                    for (const std::shared_ptr <Player> &p: players) {
                        if (!p->getIsBankrupt())
                            scoreboard->onWin(p->getName());
                    }
                    return;
                }
            }
        }
        findWinner();
    }
};

#endif
