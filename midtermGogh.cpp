#include <bangtal.h>
#include <iostream>
#include <ctime>
#include <windows.h>
#include <algorithm>

using namespace bangtal;
using namespace std;


//객체 생성
class Board
{
private:
	ObjectPtr front;
	ObjectPtr back;
public:
	void getPuzzle(string filenameF, string filenameB, ScenePtr scene, int i) {
		front = Object::create(filenameF, scene, 400 + (i % 3) * 160, 500 - (i / 3) * 160);
		back = Object::create(filenameB, scene, 400 + (i % 3) * 160, 500 - (i / 3) * 160, false);
	}
	ObjectPtr getFront() {
		return front;
	}
	ObjectPtr getBack() {
		return back;
	}
	ObjectPtr& setFront() { return front; };
	ObjectPtr& setBack() { return back; };
};
class Cards
{
private:
	ObjectPtr card;
	int CardNum;
public:
	Cards() {
		CardNum = -1;
	}
	void getCard(string cardName, ScenePtr scene, int i, int j, int arr[]) {
		card = Object::create(cardName, scene, 120 + (arr[i] % 6) * 180, 480 - (arr[i] / 6) * 225);
		CardNum = j;
	}
	ObjectPtr getCard() {
		return card;
	}
	int getCardNum() {
		return CardNum;
	}
};

// 카드 게임을 위한 연산 함수
int goghCard = 0;
int handedCard = -1;  // handedCard 가 -1이면 새로운 카드 번호 저장, 아니면 둘을 비교
int prevClickedCard;
int flipedCard[18];
bool checkFirst = true;
int flipedNum = 0;
int checkCard(int cardNum, int clickedCardNum) {
	if (checkFirst) {								//flipedCard 배열 -1으로 초기화.
		fill_n(flipedCard, 18, -1);
		checkFirst = false;
	}	
	
	bool exist = false;
	for (int i=0; i < 18; i++) {
		if (flipedCard[i] == clickedCardNum) exist = true;
	}
	if (exist) return -5;							// 이미 뒤집어진 카드를 누른 경우
	else if (handedCard == -1) {					// 첫번째 카드인 경우
		handedCard = cardNum;
		prevClickedCard = clickedCardNum;
		return -3;
	}
	else if (prevClickedCard == clickedCardNum) {	// 첫번째 카드를 누른 경우
		handedCard = -1;
		return -4;
	}
	else if (handedCard == cardNum) {				// 같은 카드를 고른 경우
		if (cardNum == 0 || cardNum == 1) {			// 고흐 카드
			showMessage("고흐의 카드를 맞추셨습니다!");
			goghCard++;
			handedCard = -1;
			flipedCard[flipedNum] = prevClickedCard;
			flipedCard[flipedNum + 1] = clickedCardNum;
			flipedNum += 2;
			return (goghCard == 2) ? -2 : -1;

		}
		else {										// 고흐 외의 카드
			showMessage("카드를 맞췄습니다! 하지만 고흐에게 도움이 안되네요..");
			handedCard = -1;
		}
		flipedCard[flipedNum] = prevClickedCard;
		flipedCard[flipedNum + 1] = clickedCardNum;
		flipedNum += 2;
		return -1;
	}
	else {											// 서로 다른 카드를 고른 경우
		handedCard = -1;
		return prevClickedCard;
	}
};

//메인 함수
int main() {
	// 불필요한 부분 삭제
	setGameOption(GameOption::GAME_OPTION_MESSAGE_BOX_BUTTON, false);
	setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, false);

	//장면들
	ScenePtr EnterScreen = Scene::create("반 고흐를 살려라", "images/Background.png");
	ScenePtr scene1 = Scene::create("퍼즐 맞추기", "images/scene.png");
	ScenePtr scene2 = Scene::create("퍼즐 맞추기", "images/scene.png");
	ScenePtr scene3 = Scene::create("카드 맞추기", "images/scene.png");
	ScenePtr FailScreen = Scene::create("게임 실패", "images/Background.png");
	ScenePtr EndScreen = Scene::create("반 고흐를 살려라", "images/Background.png");

	//소리
	SoundPtr S_success = Sound::create("sounds/Success.mp3");
	SoundPtr S_card = Sound::create("sounds/Card.mp3");
	SoundPtr S_puzzle = Sound::create("sounds/Puzzle.mp3");
	SoundPtr S_button = Sound::create("sounds/Button.mp3");

	///////////////////////////
	///        INTRO        ///
	///////////////////////////
	auto gogh = Object::create("images/VanGogh.png", EnterScreen, 0, 0);
	auto start = Object::create("images/start.png", EnterScreen, 900, 250);
	auto logo = Object::create("images/logo.png", EnterScreen, 400, 400);

	showMessage("도저히 영감이 떠오르지 않아.. 나를 도와줘");
	//첫번째 게임으로 이동
	start->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action)->bool {
		S_button->play(false);
		scene1->enter();
		showMessage("2개의 뒤집어지는 퍼즐을 5분안에 맞춰 고흐에게 영감을 주세요!");
		return true;
		});

	///////////////////////////
	///  첫 번째 게임 부분  ///
	///  양면 퍼즐 맞추기   ///
	///////////////////////////

	//랜덤 초기화
	srand((unsigned int)time(NULL));
	auto timerRandom = Timer::create(0.1f);

	//게임 설명 부분 - 퍼즐 맞추기
	auto paint1 = Object::create("images/full_a.png", scene1, 100, 200);
	auto paint2 = Object::create("images/full_b.png", scene1, 700, 200);
	auto startPuzzle1 = Object::create("images/start.png", scene1, 500, 50);


	// 첫번째 게임 타이머 설정
	auto timer = Timer::create(300.0f);
	timer->setOnTimerCallback([&](TimerPtr)->bool {
		showMessage("그림 완성 실패!!");
		FailScreen->enter();
		return true;
		});
	// 두번째 게임 타이머 설정
	auto timerCard = Timer::create(120.0f);
	timerCard->setOnTimerCallback([&](TimerPtr)->bool {
		showMessage("카드 찾기 실패!!");
		FailScreen->enter();
		return true;
		});
	//게임 시작 버튼
	startPuzzle1->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action)->bool {
		S_button->play(false);
		scene2->enter();
		timerRandom->start();
		timer->start();
		showTimer(timer);
		return true;
		});

	//게임 부분 - 퍼즐 맞추기
	auto retry = Object::create("images/start.png", scene2, 500, 50, false);
	auto next = Object::create("images/next.png", scene2, 800, 50, false);
	auto logo2 = Object::create("images/logo.png", scene2, 250, 50);

	//두번째 게임으로 넘어가는 Next 버튼 (HIDE 상태)
	next->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action) -> bool {
		S_button->play(false);
		scene3->enter();
		showMessage("같은 카드를 찾으세요! 고흐의 그림 두개를 2분 안에 찾으셔야 합니다.");
		timerCard->start();
		showTimer(timerCard);
		return true;
		});
	ObjectPtr init_board_front[9];
	ObjectPtr init_board_back[9];
	int blank = 8;
	Board board[9];

	for (int i = 0; i < 9; i++) {
		string filenameA = "images/" + to_string(i + 1) + "a.png";
		string filenameB = "images/" + to_string(i + 1) + "b.png";
		board[i].getPuzzle(filenameA, filenameB, scene2, i);

		init_board_front[i] = board[i].getFront();
		init_board_back[i] = board[i].getBack();
		/// 앞면일때
		board[i].getFront()->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction)->bool {
			int j;
			S_puzzle->play(false);
			ObjectPtr tmpF, tmpB;

			for (j = 0; j < 9; j++) {
				if (board[j].getFront() == object)
					break;
			}

			if ((j % 3 > 0 && j - 1 == blank) ||
				(j % 3 < 2 && j + 1 == blank) ||
				(j < 6 && j + 3 == blank) ||
				(j > 2 && j - 3 == blank)) {
				board[j].getFront()->locate(scene2, 400 + (blank % 3) * 160, 500 - (blank / 3) * 160);
				board[blank].getFront()->locate(scene2, 400 + (j % 3) * 160, 500 - (j / 3) * 160);
				board[j].getBack()->locate(scene2, 400 + (blank % 3) * 160, 500 - (blank / 3) * 160);
				board[blank].getBack()->locate(scene2, 400 + (j % 3) * 160, 500 - (j / 3) * 160);


				tmpF = board[j].getFront();
				board[j].setFront() = board[blank].getFront();
				board[blank].setFront() = tmpF;
				tmpB = board[j].getBack();
				board[j].setBack() = board[blank].getBack();
				board[blank].setBack() = tmpB;

				blank = j;

				for (int x = 0; x < 9; x++) {
					board[x].getFront()->hide();
					board[x].getBack()->show();
				}

				board[j].getBack()->hide();



				int k;
				for (k = 0; k < 9; k++) {
					if (board[k].getFront() != init_board_front[k]) break;
				}
				if (k == 9) {
					showMessage("그림을 완성시켜줘서 고마워!");
					timer->stop();
					next->show();
				}
			}

			return true;
			});
		/// 뒷면일때
		board[i].getBack()->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction)->bool {
			S_puzzle->play(false);
			int j;
			ObjectPtr tmpF, tmpB;

			for (j = 0; j < 9; j++) {
				if (board[j].getBack() == object)
					break;
			}

			if ((j % 3 > 0 && j - 1 == blank) ||
				(j % 3 < 2 && j + 1 == blank) ||
				(j < 6 && j + 3 == blank) ||
				(j > 2 && j - 3 == blank)) {
				board[j].getBack()->locate(scene2, 400 + (blank % 3) * 160, 500 - (blank / 3) * 160);
				board[blank].getBack()->locate(scene2, 400 + (j % 3) * 160, 500 - (j / 3) * 160);
				board[j].getFront()->locate(scene2, 400 + (blank % 3) * 160, 500 - (blank / 3) * 160);
				board[blank].getFront()->locate(scene2, 400 + (j % 3) * 160, 500 - (j / 3) * 160);


				tmpB = board[j].getBack();
				board[j].setBack() = board[blank].getBack();
				board[blank].setBack() = tmpB;
				tmpF = board[j].getFront();
				board[j].setFront() = board[blank].getFront();
				board[blank].setFront() = tmpF;

				blank = j;


				for (int x = 0; x < 9; x++) {
					board[x].getBack()->hide();
					board[x].getFront()->show();
				}
				board[j].getFront()->hide();


				int k;
				for (k = 0; k < 9; k++) {
					if (board[k].getBack() != init_board_back[k]) break;
				}
				if (k == 9) {
					showMessage("그림을 완성시켜줘서 고마워!");
					timer->stop();
					next->show();
				}
			}

			return true;
			});
	}
	board[blank].getFront()->hide();
	board[blank].getBack()->hide();

	auto count = 0;
	timerRandom->setOnTimerCallback([&](TimerPtr t)->bool {
		//Random하게 blank 움직이기
		int j = 0;
		switch (rand() % 4) {
		case 0:j = blank - 1; break;
		case 1:j = blank + 1; break;
		case 2:j = blank - 3; break;
		case 3:j = blank + 3; break;
		}
		if ((j > 0 && j < 9) && ((j % 3 > 0 && j - 1 == blank) ||
			(j % 3 < 2 && j + 1 == blank) ||
			(j < 6 && j + 3 == blank) ||
			(j > 2 && j - 3 == blank)))
		{
			board[j].getFront()->locate(scene2, 400 + (blank % 3) * 160, 500 - (blank / 3) * 160);
			board[blank].getFront()->locate(scene2, 400 + (j % 3) * 160, 500 - (j / 3) * 160);
			board[j].getBack()->locate(scene2, 400 + (blank % 3) * 160, 500 - (blank / 3) * 160);
			board[blank].getBack()->locate(scene2, 400 + (j % 3) * 160, 500 - (j / 3) * 160);

			//앞면
			auto tmp = board[j].getFront();
			board[j].setFront() = board[blank].getFront();
			board[blank].setFront() = tmp;
			// 뒷면
			tmp = board[j].getBack();
			board[j].setBack() = board[blank].getBack();
			board[blank].setBack() = tmp;
			blank = j;
			count++;
		}

		if (count < 50) {
			t->set(0.02f);
			t->start();
		}
		return true;
		});

	///////////////////////////
	///  두 번째 게임 부분  ///
	///  그림 카드 맞추기   ///
	///////////////////////////
	auto timerFlip = Timer::create(0.8f);

	Cards card[18];
	int Shuffle[18];
	for (int i = 0; i < 18; i++) {
		Shuffle[i] = i;
	}
	for (int i = 0; i < 80; i++) {
		int S1 = rand() % 18;
		int S2 = rand() % 18;

		int tmp = Shuffle[S1];
		Shuffle[S1] = Shuffle[S2];
		Shuffle[S2] = tmp;
	}

	for (int i = 0; i < 9; i++) {
		card[i].getCard("images/cardBack.png", scene3, i, i, Shuffle);
		card[i + 9].getCard("images/cardBack.png", scene3, i + 9, i, Shuffle);
	}



	int clickedObj;
	for (int i = 0; i < 18; i++) {
		card[i].getCard()->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction)->bool {
			// clickedObj = 클릭 된 카드의 숫자 
			for (int j = 0; j < 18; j++) {
				if (card[j].getCard() == object) {
					clickedObj = j;
				}
			}
			// 카드 뒤집기
			string openCardString = "images/card" + to_string(card[clickedObj].getCardNum() + 1) + ".png";
			card[clickedObj].getCard()->setImage(openCardString);

			// 뒤집은 카드들 연산
			int resCheckCard = checkCard(card[clickedObj].getCardNum(), clickedObj);               // 결과값 -1: 뒤집기 성공 | -2: 성공 | -3: 첫번째 카드 | -4: 같은 카드 클릭| 나머지 : PREV값
			if (!(resCheckCard == -4 || resCheckCard == -5)) {
				S_card->play(false);
			}
			if (resCheckCard > -1) {
				showMessage("둘이 다른 그림입니다.");
				timerFlip->setOnTimerCallback([&](TimerPtr t)->bool {             //뒤집기 위한 시간, 두 사진을 다 flip 하면 오류 발생.
					card[clickedObj].getCard()->setImage("images/cardBack.png");
					t->set(0.8f);
					return true;
					});
				timerFlip->start();
				card[resCheckCard].getCard()->setImage("images/cardBack.png");

			}
			else if (resCheckCard == -2) {
				S_success->play(false);
				timerCard->stop();
				EndScreen->enter();
			}

			return true;
			});
		///////////////////////////
		///     엔딩  부분      ///
		///////////////////////////
		auto goghEnd = Object::create("images/VanGogh.png", EndScreen, 0, 0);
		auto goghTalk = Object::create("images/goghTalk.png", EndScreen, 380, 400);
		auto endEndScreen = Object::create("images/end.png", EndScreen, 800, 50);
		endEndScreen->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action)->bool {
			S_button->play(false);
			endGame();
			return true;
			});

		goghTalk->setScale(0.8f);

		///////////////////////////
		///     실패  부분      ///
		///////////////////////////
		auto goghFail = Object::create("images/VanGogh.png", FailScreen, 0, 0);
		auto goghFailTalk = Object::create("images/goghFailTalk.png", FailScreen, 380, 400);
		auto endFailScreen = Object::create("images/end.png", FailScreen, 800, 50);
		endFailScreen->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction action)->bool {
			S_button->play(false);
			endGame();
			return true;
			});

		goghFailTalk->setScale(0.8f);

	}
	//게임 시작
	startGame(EnterScreen);
	return 0;
}

