#include<iostream>
#include<ctime>
#include<vector>
#include<thread>
#include<conio.h>
using namespace std;

const int fieldWidth = 32; // the width of the game field including borders
const int fieldHeight = 32; // the height of the game field including borders
int score; // game score
int lives; // lives of the hero
time_t beginTime; // start time of the game
bool runGame; // state of the game (running or not)
int goldX, goldY;

class Object // base class for Bullets and Tanks
{
protected:
	char direction; // direction to move (and shoot)
	int x, y; // coordinates
	char sign; // the sign that is shown on the field (for bullet - *, for enemy - 0, for hero - x)
	bool isHero;

public:
	Object()
	{
		direction = 'u';
		x = fieldWidth / 2;
		y = fieldHeight / 2;
		sign = '*';
		isHero = false;
	}
	virtual bool Move(char **field) // function to move object
	{
		bool flag = false;
		int X = x, Y = y;
		if (!isHero && rand() % 6 == 0) // change direction for enemies randomly
		{
			char dir;
			do{
				switch (rand() % 4)
				{
				case 0:
					dir = 'u'; // UP
					break;
				case 1:
					dir = 'd'; // DOWN
					break;
				case 2:
					dir = 'l'; // LEFT
					break;
				case 3:
					dir = 'r'; // RIGHT
					break;
				default:
					break;
				}
			} while (dir == direction);
			SetDirection(dir);
		}
		switch (direction) // changes coodrinates depending on direction
		{
		case 'u': Y--; flag = (field[x][y - 1] == ' '); break;
		case 'd': Y++; flag = (field[x][y + 1] == ' '); break;
		case 'r': X++; flag = (field[x + 1][y] == ' '); break;
		case 'l': X--; flag = (field[x - 1][y] == ' '); break;
		default:
			break;
		}
		if (flag)
		{
			field[x][y] = ' ';
			x = X;
			y = Y;
			field[x][y] = sign;
			return true;
		}
		else if (field[X][Y] == 'G')
		{
			field[x][y] = ' ';
			x = X;
			y = Y;
			field[x][y] = sign;
			return true;
		}
		else
		{
			return false;
		}
	}
	int GetX()
	{
		return x;
	}
	int GetY()
	{
		return y;
	}
	char GetSign()
	{
		return sign;
	}
	char GetDirection()
	{
		return direction;
	}
	void SetX(int x)
	{
		this->x = x;
	}
	void SetY(int y)
	{
		this->y = y;
	}
	void SetSign(char sign)
	{
		this->sign = sign;
	}
	void SetDirection(char dir)
	{
		if (dir == 'u' || dir == 'd' || dir == 'l' || dir == 'r')
			direction = dir;
	}
	void SetHero(bool isHero)
	{
		this->isHero = isHero;
	}
	bool IsHero()
	{
		return isHero;
	}
};

class Bullet : public Object
{
public:
	Bullet()
	{
		Object();
		sign = '*';
	}
	bool Move(char **field) // moves the bullet
	{
		bool flag = true;
		int X = x, Y = y;
		switch (direction) //change coordinates depending on direction
		{
		case 'u': Y -= 2; // UP
			for (int k = y - 1; k >= 0 && k >= Y; k--)
			{
				if (field[x][k] != ' ')
				{
					Y = k;
					flag = false;
					break;
				}
			}
			break;
		case 'd': Y += 2; // DOWN
			for (int k = y + 1; k < fieldHeight && k <= Y; k++)
			{
				if (field[x][k] != ' ')
				{
					Y = k;
					flag = false;
					break;
				}
			}
			break;
		case 'r': X += 2; // RIGHT
			for (int k = x + 1; k < fieldWidth && k <= X; k++)
			{
				if (field[k][y] != ' ')
				{
					X = k;
					flag = false;
					break;
				}
			}
			break;
		case 'l': X -= 2; // LEFT
			for (int k = x - 1; k >= 0 && k >= X; k--)
			{
				if (field[k][y] != ' ')
				{
					X = k;
					flag = false;
					break;
				}
			}
			break;
		default:
			break;
		}
		if (field[x][y] == this->GetSign())
			field[x][y] = ' ';
		x = X;
		y = Y;
		if (flag)
		{
			field[x][y] = sign;
			return true;
		}
		else
		{
			return false;
		}
	}
};

class Tank : public Object
{
public:
	Tank()
	{
		Object();
		sign = '0';
	}
	Bullet Shoot() // "shooting" - creates new Bullet
	{
		Bullet b;
		b.SetDirection(this->GetDirection());
		b.SetX(this->GetX());
		b.SetY(this->GetY());
		b.SetHero(this->IsHero());
		return b;
	}
};

// function for moving bullets and enemies
void MoveObjects(char **field, vector<Tank> &enemies, vector<Bullet> &bullets, Tank &hero)
{
	for (int i = 0; i < bullets.size(); i++)
	{
		bool result = bullets[i].Move(field);
		if (!result) // if bullet hits some aim
		{
			int bullX = bullets[i].GetX();
			int bullY = bullets[i].GetY();
			if (bullX != 0 && bullY != 0 && bullX != fieldWidth - 1 && bullY != fieldHeight - 1){
				if (field[bullX][bullY] == '#')
				{
					field[bullX][bullY] = ' ';
				}
				else if (field[bullX][bullY] != 'G')
				{
					bool found = false;
					for (int k = 0; k < enemies.size(); k++)
					{
						if (enemies[k].GetX() == bullets[i].GetX() && enemies[k].GetY() == bullets[i].GetY()) // check whether bullet hits enemy
						{
							found = true;
							if (bullets[i].IsHero()) // if bullet is from hero - destroy enemy
							{
								field[enemies[k].GetX()][enemies[k].GetY()] = ' ';
								enemies.erase(enemies.begin() + k);
								score++;
								if (enemies.size() == 0)
									runGame = false;
							}
							break;
						}
					}
					if (!found)
					{ // check whether the bullet hits the hero
						if (bullets[i].GetX() == hero.GetX() && bullets[i].GetY() == hero.GetY())
						{
							if (!bullets[i].IsHero()) // if bullet is from enemy -> lives--
							{
								lives--;
								if (lives <= 0)
									runGame = false;
							}
						}
					}
				}
			}
			bullets.erase(bullets.begin() + i);
			i--;
		}
	}
	for (int i = 0; i < enemies.size(); i++)
	{
		enemies[i].Move(field);
		if (enemies[i].GetX() == goldX && enemies[i].GetY() == goldY)
			runGame = false;
	}
}

void AddGold(char **field)
{
	goldX = fieldWidth / 2;
	goldY = fieldHeight - 2;
	for (int i = goldX - 1; i <= goldX + 1; i++)
		for (int j = goldY - 1; j <= goldY; j++)
			field[i][j] = '#';
	field[goldX][goldY] = 'G';
}

void AddObstacles(char **field) // adding obstacles on the field
{
	srand(time(0));
	int obstNumber = (rand() % 4) + 8;
	for (int ob = 0; ob < obstNumber; ob++)
	{
		int x = rand() % fieldWidth;
		int y = rand() % fieldHeight;
		int dir = rand() & 1;
		int len = (rand() % 5) + 6;
		if (dir)
		{
			for (int pos = x, k = 0; k < len && pos < fieldWidth; pos++, k++)
				if (field[pos][y] == ' ')
					field[pos][y] = '#';
		}
		else
		{
			for (int pos = y, k = 0; k < len && pos < fieldHeight; pos++, k++)
				if (field[x][pos] == ' ')
					field[x][pos] = '#';
		}
	}
}

void AddEnemies(char **field, vector<Tank> &enemies, Tank &hero) // adding enemies
{
	srand(time(0));
	int enemiesNumber = (rand() % 3) + 4; // the number of enemies from 4 to 6
	for (int enemy = 0; enemy < enemiesNumber; enemy++)
	{
		int x, y;
		bool flag = true;
		while (flag){
			flag = false;
			x = rand() % fieldWidth;  // randomly placing the enemies
			y = rand() % fieldHeight; // on the field
			if (field[x][y] != ' ')
			{
				flag = true;
				continue;
			}
			else
			{
				for (int i = 0; i < enemies.size(); i++) // checking if the distance between tanks if more than 2 cells
				{
					if (abs(enemies[i].GetX() - x) < 3 || abs(enemies[i].GetY() - y) < 3)
					{
						flag = true; 
						break;
					}
				}
				if (!flag)
				{
					if (abs(hero.GetX() - x) < 3 || abs(hero.GetY() - y) < 3)
					{
						flag = true;
					}
				}
			}
		}
		char direction;
		switch (rand()%4) // setting the direction
		{
		case 0: 
			direction = 'u'; break;
		case 1:
			direction = 'd'; break;
		case 2:
			direction = 'r'; break;
		case 3:
			direction = 'l'; break;
		default:
			break;
		}
		Tank t;
		t.SetX(x);
		t.SetY(y);
		t.SetDirection(direction);
		t.SetHero(false);
		enemies.push_back(t);
		field[x][y] = enemies[enemy].GetSign();
	}
}

void AddHero(char **field, Tank &hero) // creating hero and placing it on the field
{
	int x = fieldWidth / 2 + 3, y = fieldHeight - 3;
	hero.SetX(x);
	hero.SetY(y);
	hero.SetDirection('u');
	hero.SetHero(true);
	hero.SetSign('x');
	field[x][y] = hero.GetSign();
}

void DrawField(char **field) // draws the game field
{
	if (field[goldX][goldY] == ' ')
		field[goldX][goldY] = 'G';
	for (int y = 0; y < fieldHeight; y++)
	{
		for (int x = 0; x < fieldWidth; x++)
			cout << field[x][y];
		cout << "\n";
	}
	cout << "\nScore: " << score << endl;
	cout << "Lives: " << lives << endl;
	cout << "Time: " << time(0) - beginTime << endl;
}

void Shooting(vector<Tank> &enemies, vector<Bullet> &bullets) // random shooting of enemies
{
	for (int i = 0; i < enemies.size(); i++)
	{
		if (rand() % 5 == 0)
			bullets.push_back(enemies[i].Shoot());
	}
}

void MoveHero(char **field, Tank &hero, vector<Bullet> &bullets) // move hero or shoot depending on the keystroke
{
	if (_kbhit())
	{
		int key = _getch();
		if (key == 32)
		{
			bullets.push_back(hero.Shoot());
			field[hero.GetX()][hero.GetY()] = hero.GetSign();
		}
		else if (key == 224)
		{
			key = _getch();
			switch (key)
			{
			case 75: //left 
				hero.SetDirection('l');
				break;
			case 77: //right
				hero.SetDirection('r');
				break;
			case 72: //up
				hero.SetDirection('u');
				break;
			case 80: //down
				hero.SetDirection('d');
				break;
			default:
				break;
			}
			hero.Move(field);
		}
	}

}

void GameLoop(char **field, vector<Tank> &enemies, vector<Bullet> &bullets, Tank &hero)
{
	while (runGame)
	{
		MoveHero(field, hero, bullets);
		Shooting(enemies, bullets);
		MoveObjects(field, enemies, bullets, hero);
		system("cls");
		DrawField(field);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if (enemies.size() > 0)
		cout << "\nGame over!\n";
	else
		cout << "\nYou win!!!\n";
}

int main()
{
	score = 0;
	lives = 3;
	runGame = true;
	char **field = new char*[fieldWidth];
	for(int i = 0; i < fieldWidth; i++)
		field[i] = new char[fieldHeight];
	for (int x = 0; x < fieldWidth; x++)
	{
		field[x][0] = '#';
		field[x][fieldHeight - 1] = '#';
	}
	for (int y = 1; y < fieldHeight - 1; y++)
	{
		field[0][y] = '#';
		for (int x = 1; x < fieldWidth - 1; x++)
			field[x][y] = ' ';
		field[fieldWidth - 1][y] = '#';
	}
	vector<Tank> enemies;
	Tank hero;
	vector<Bullet> bullets;
	AddGold(field);
	AddHero(field, hero);
	AddObstacles(field);
	AddEnemies(field, enemies, hero);
	beginTime = time(0);
	DrawField(field);
	GameLoop(field, enemies, bullets, hero);
	cin.sync();
	cin.get();
	for (int i = 0; i < fieldWidth; i++)
		delete[] field[i];
	delete[] field;
	return 0;
}
