#include <iostream>
#include <Windows.h>
#include <string>
#include <cmath>
#include <fstream>

struct Edge {//Список ребер
	int v;
	int w;
	double length;
	Edge* next;
	Edge(int v, int w, double l) {
		this->v = v;
		this->w = w;
		this->length = l;
		this->next = nullptr;
	}
	Edge() {}
};

struct curCycle {//Цикл, який повертається у результаті роботи функції isCycle (фіксація початку і відповідно кінця циклу)
	//та isCycleV (проміжні вершини). Стек
	int v;
	curCycle* next;
	curCycle(int v) {
		this->v = v;
		this->next = nullptr;
	}
};

struct Cycle3 {//Список циклів довжини 3
	double s; //площа
	int* v; //вершини
	Cycle3* next;
	Cycle3(double s, int v[]) {
		this->s = s;
		this->v = v;
		this->next = nullptr;
	}
};

double Square(Edge sides[]) {//обчислення площі за формулою Герона
	double halfperimeter = (sides[0].length + sides[1].length + sides[2].length) / 2;
	double h0 = halfperimeter - sides[0].length;
	double h1 = halfperimeter - sides[1].length;
	double h2 = halfperimeter - sides[2].length;
	double product = halfperimeter * h0 * h1 * h2;
	return sqrt(product);
}

void shell_sort(Cycle3 **c, int size) {
	for (int s = size / 2; s > 0; s /= 2) {
		for (int i = s; i < size; ++i) {
			for (int j = i - s; j >= 0 && c[j]->s < c[j + s]->s; j -= s) {
				Cycle3* temp = c[j];
				c[j] = c[j + s];
				c[j + s] = temp;
			}
		}
	}
}

bool triangle_exists(Edge csides[3]) {
	//Перевірка існування трикутника із довжинами ребер csides за правилом:
	//Сума будь-яких двох сторін трикутника є меншою за довжину третьої.
	if (csides[0].length >= csides[1].length + csides[2].length) {
		return false;
	}
	if (csides[1].length >= csides[0].length + csides[2].length) {
		return false;
	}
	if (csides[2].length >= csides[0].length + csides[1].length) {
		return false;
	}
	return true;
}

void addCycle(Cycle3 *&list, curCycle *curc, Edge* elist) {
	int* cv = new int[3]; //Масив вершин
	cv[0] = curc->v;
	cv[1] = curc->next->v;
	cv[2] = curc->next->next->v;
	Edge csides[3];//0 - з'єднує 0 і 1, 1 - 1 і 2, 2 - 0 і 2
	Edge* eiter = elist;
	while (eiter) {
		if (eiter->v == cv[0] && eiter->w == cv[1] || eiter->v == cv[1] && eiter->w == cv[0])
		{
			csides[0].v = eiter->v;
			csides[0].w = eiter->w;
			csides[0].length = eiter->length;
		}
		else if (eiter->v == cv[1] && eiter->w == cv[2] || eiter->v == cv[2] && eiter->w == cv[1])
		{
			csides[1].v = eiter->v;
			csides[1].w = eiter->w;
			csides[1].length = eiter->length;
		}
		else if (eiter->v == cv[0] && eiter->w == cv[2] || eiter->v == cv[2] && eiter->w == cv[0])
		{
			csides[2].v = eiter->v;
			csides[2].w = eiter->w;
			csides[2].length = eiter->length;
		}
		eiter = eiter->next;
	}
	double s;
	if (triangle_exists(csides))
		s = Square(csides);
	else
		s = -1;//Якщо трикутнику зі сторонами csides[i].length не існує, записуємо площу як -1
	Cycle3* cyc = new Cycle3(s, cv);
	Cycle3* citer = list;
	if (!citer)
	{
		list = new Cycle3(s, cv);
	}
	else {
		while (citer->next) {
			citer = citer->next;
		}
		citer->next = cyc;
		citer = cyc;
	}
}

void findCycle(int M, int** adj, bool* visited, int len, int v, int start, Cycle3 *&crc, curCycle* curc, Edge* elist) {
	visited[v] = true;//Позначаємо вершину як відвідану
	if (!curc)
		curc = new curCycle(v);//Заносимо вершину у простий ланцюг curc
	else
	{
		curCycle* cyc = new curCycle(v);
		cyc->next = curc;
		curc = cyc;
	}
	if (!len) {//len - кількість ребер, що залишились, якими треба пройти, щоб потім зробити висновок, чи можна додати 
		//до простого ланцюга curc вершину start (тобто, з якої починався пошук в глибину), щоб отримати цикл
		//Якщо вже не залишилось таких ребер:
		visited[v] = false;
		if (adj[v][start])//Перевіряємо суміжність вершини, на якій ми зупинились, із стартовою вершиною
		{
			addCycle(crc, curc, elist);//І якщо вершини суміжні, заносимо простий ланцюг crc у список циклів
			return;
		}
		else
			return;
	}
	for (int i = 0; i < M; i++) {
		if (!visited[i] && adj[v][i])//По кожній вершині, яка не була відвідана, і яка суміжна з теперішньою вершиною, продовжуємо пошук в глибину
			findCycle(M, adj, visited, len - 1, i, start, crc, curc, elist);
	}
	//По завершенню циклу робимо поп із стеку curc, позначаємо вершину v, як ту, що ще можна відвідати, і повертаємось з виклику
	curCycle* a = curc;
	curc = curc->next;
	delete a;
	visited[v] = false;
}

bool notDub(int* a, int asize, int b) {
	//Перевірка, чи не містить масив, куди записані індекси унікальних циклів, число b
	for (int i = 0; i < asize; i++) {
		if (a[i] == b)
			return false;
	}
	return true;
}

void findAndSortCycles(int M, Edge* elist, int** adj) {
	bool* visited = new bool[M];//Масив відвіданих вершин. Якщо вершина позначена, як відвідана, то при пошуку у глибину не відбувається переходу на неї
	for (int i = 0; i < M; i++) {
		visited[i] = false;
	}
	Cycle3* cyclelist = nullptr;//Список циклів довжиною 3
	for (int i = 0; i < M - 2; i++) {
		findCycle(M, adj, visited, 2, i, i, cyclelist, nullptr, elist);//Знаходимо цикл із кінцями у вершині i пошуком у глибину
		visited[i] = true;
	}

	//Перетворюємо список в матрицю для подальшого її сортування
	int cn = 0;
	Cycle3* citer = cyclelist;
	while (citer) {
		cn++;
		citer = citer->next;
	}
	Cycle3** cm = new Cycle3*[cn];
	citer = cyclelist;
	int i = 0;
	while (citer) {
		cm[i] = new Cycle3(citer->s, citer->v);
		i++;
		citer = citer->next;
	}
	//У списку один й той самий цикл записується два рази, бо маршрут, за яким цикл і визначається у findCycle, будується в обох напрямках.
	//Потрібно позбавитись від дублікатів.
	int cn1 = cn / 2;
	Cycle3** cm1 = new Cycle3*[cn1];
	int c = 0;
	int* dubl = new int[cn];
	for (i = 0; i < cn; i++)
		dubl[i] = -1;
	for (i = 0; i < cn; i++)
	{
		if (notDub(dubl, cn, i))
			for (int j = 0; j < cn; j++) {
				bool sym = true;
				//Кінцями циклічного маршруту є останній, тобто третій елемент матриці вершин у циклі.
				//У дублікатів циклу (назвемо їх Х та У) 1-й елемент елемент матриці вершин циклу Х дорівнює 2-му елементу м. в. циклу У і навпаки,
				//2-й ел. м. в. циклу Х = 1-й ел. м. в. циклу У
				//В новий масив циклів записуємо лише один такий дублікат, а другий відкидаємо.
				if (notDub(dubl, cn, j))
				{
					for (int k = 0; k < 2; k++) {
						if (cm[i]->v[k] != cm[j]->v[1 - k])
						{
							sym = false;
							break;
						}
					}
					if (sym) {
						cm1[c] = new Cycle3(cm[i]->s, cm[i]->v);
						dubl[2*c] = i;
						dubl[2*c+1] = j;
						c++;
						break;
					}
				}
			}
	}
	for (i = 0; i < cn1; i++) {
		std::cout << "Цикл " << i + 1 << ":\n";
		std::cout << "\t- Вершини: " << cm1[i]->v[0] << ", " << cm1[i]->v[1] << ", " << cm1[i]->v[2] << "\n";
		cm1[i]->s == -1 ?
			std::cout << "\t- Трикутника із довжинами ребер, що входять до циклу, не існує" :
			std::cout << "\t- Площа: " << floor(cm1[i]->s * 100 + 0.5) / 100 << "\n";
	}

	shell_sort(cm1, cn1);
	std::cout << "\nВідсортовані за площею трикутника: \n";
	for (i = 0; i < cn1; i++) {
		std::cout << "Цикл " << i + 1 << ":\n";
		std::cout << "\t- Вершини: " << cm1[i]->v[0] << ", " << cm1[i]->v[1] << ", " << cm1[i]->v[2] << "\n";
		cm1[i]->s == -1 ?
			std::cout << "\t- Трикутника із довжинами ребер, що входять до циклу, не існує" :
			std::cout << "\t- Площа: " << floor(cm1[i]->s * 100 + 0.5) / 100 << "\n";
	}
}

int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	std::cout << "Виявлення і сортування циклів довжиною 3 у графі за площею трикутників, що вони утворюють\n\n";

	//Граф подається матрицею суміжності, але якщо вершини суміжні, то у віповідній клітинці записується
	//довжина ребра, що їх з'єднує

	std::ifstream file("graph.txt");
	if (!file)
		std::cout << "Файл graph.txt не виявлено. Створіть його і спробуйте ще раз.\n";
	else {
		std::string row;
		int M = 0;
		while (!file.eof())
		{
			std::getline(file, row);
			M++;
		}
		file.seekg(0, file.beg);
		int** AdjM = new int*[M];
		for (int i = 0; i < M; i++) {
			AdjM[i] = new int[M];
		}
		for (int i = 0; i < M; i++) {//Зчитуємо матрицю суміжності розміру M*M
			std::getline(file, row);
			std::string s = "";
			int j = 0;
			int rl = row.length();
			for (int k = 0; k < rl; k++) {
				if (row[k] != ' ')
					s += row[k];
				if (row[k] == ' ' || k == rl - 1) {
					AdjM[i][j] = std::stoi(s);
					s = "";
					j++;
				}
			}
		}
		/*for (int i = 0; i < M; i++) {
			for (int j = 0; j < M; j++)
				std::cout << AdjM[i][j] << " ";
			std::cout << "\n";
		}*/
		Edge* elist = nullptr;//Ребра
		Edge* etail = nullptr;
		for (int i = 0; i < M; i++) {
			for (int j = i; j < M; j++) {//Так як матриця суміжності неорієнтованого графу є симетричною, то робимо запис ребер у список, зчитуючи лише область справа
				//від головної діагоналі
				if (AdjM[i][j])
				{
					Edge* e = new Edge(i, j, AdjM[i][j]);
					if (!elist)
					{
						elist = e;
						etail = e;
					}
					else {
						etail->next = e;
						etail = e;
					}
				}
			}
		}
		Edge* eiter = elist;
		std::cout << "Ребра графу: \n";
		while (eiter) {
			std::cout << "(" << eiter->v << ", " << eiter->w << ")\n";
			eiter = eiter->next;
		}

		findAndSortCycles(M, elist, AdjM);//Переходимо до пошуку циклів графу
	}
	return 0;
}
