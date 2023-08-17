#include <iostream>
#include <string>
#include <numeric>
#include <vector>
#include <memory>
#include <fstream>
#include <regex>
#include <tuple>
#include <iomanip>
#include <algorithm>

using namespace std;

// Node Class
class Node {
private:
	string name, data;
	vector<shared_ptr<Node>> nodes;
public:
	Node();
	Node(string, string);
	void setName(string);
	void setData(string);
	string getName() const;
	string getData() const;
	void appendNodes(string, string);
	vector<shared_ptr<Node>> getNodes() const;
};

// Node Constructor
Node::Node() {
	name = "";
	data = "";
}

// Node Parametrized Constructor
Node::Node(string name, string data) {
	this->name = name;
	this->data = data;
}

// Name Setter
void Node::setName(string name) {
	this->name = name;
}

// Data Setter
void Node::setData(string data) {
	this->data = data;
}

// Name Getter
string Node::getName() const {
	return this->name;
}

// Data Getter
string Node::getData() const {
	return this->data;
}

// Appending/pushing new nodes into vector
void Node::appendNodes(string name, string data) {
	shared_ptr<Node> ptr(new Node(name, data));
	nodes.push_back(ptr);
}

// Vector<shared_ptr<Node>> Getter
vector<shared_ptr<Node>> Node::getNodes() const {
	return this->nodes;
}

// Parser Class to read files
class Parser {
private:
	string fileName;
	vector<Node> vData;
	regex title;
	regex link;
	regex begin;
	regex end;
public:
	Parser();
	Parser(string);
	void parse();
	vector<Node> getvData() const;
};

// Constructor
Parser::Parser() {
	fileName = "";
}

// Parser Parametrized Constructor
Parser::Parser(string fileName) {
	this->fileName = fileName;
	// Assigning regex
	this->title = "<title>(.*)</title>";
	this->link = "<li><a href=\"(.*)\">(.*)</a></li>";
	this->begin = "<div(.*)=\"(.*)\">";
	this->end = "</div>";
}

// Parse given file while getting data with regex
void Parser::parse() {
	fstream file(fileName);
	if (file) {
		string line;
		smatch match;
		while (!file.eof()) {
			getline(file, line);
			if (regex_search(line, match, title)) {
				Node node("title", match[1]);
				getline(file, line);
				if (regex_search(line, match, begin)) {
					node.appendNodes(match[1], match[2]);
				}
				do {
					getline(file, line);
					if (regex_search(line, match, link)) {
						node.getNodes()[0]->appendNodes(match[2], match[1]);
					}
				} while (!regex_search(line, match, end));
				auto n = move(node);
				vData.push_back(n);
			}
		}
		file.close();
	}
	else cerr << "Error reading file!" << endl;
}

// Getter for vector of nodes
vector<Node> Parser::getvData() const {
	return this->vData;
}

// Graph Class
class Graph {
private:
	vector<string> names;
	vector<vector<int>> graph;
public:
	vector<string> getNames() const;
	vector<vector<int>> getGraph() const;
	void build(const vector<Node>&);
};

// Names Getter
vector<string> Graph::getNames() const {
	return this->names;
}

// Graph Getter
vector<vector<int>> Graph::getGraph() const {
	return this->graph;
}

// Build Graph from vector of nodes
void Graph::build(const vector<Node>& vData) {
	for_each(vData.begin(), vData.end(), [&](Node i) {
		names.push_back(i.getData());
		});

	graph.resize(names.size());
	for_each(graph.begin(), graph.end(), [&](auto& i) {
		i.resize(names.size(), 0);
		});

	int row{ 0 };
	for_each(vData.begin(), vData.end(), [&](Node i) {
		vector<shared_ptr<Node>> nodes = i.getNodes()[0]->getNodes();
		for_each(nodes.begin(), nodes.end(), [&](auto i) {
			vector<string>::iterator it = find_if(names.begin(), names.end(), [=](auto j) {
				transform(j.begin(), j.end(), j.begin(), toupper);
				string n = i->getName();
				transform(n.begin(), n.end(), n.begin(), toupper);
				return j == n; });
			if (it != names.end())
			{
				int col = it - names.begin();
				graph[row][col] = 1;
			}
			});
		row++;
		});
}

// PageRank class
class PageRank {
private:
	vector<tuple<string, double, int>> pageRanks;
	vector<vector<int>> vData;
	double error;
	double dampingFactor;
	double calcPRsums() {
		vector<double> differences;
		int col{ 0 };
		for_each(pageRanks.begin(), pageRanks.end(), [&](tuple<string, double, int>& t) {
			double pre = get<1>(t);
			int row{ 0 };
			get<1>(t) = accumulate(pageRanks.begin(), pageRanks.end(), 1 - dampingFactor, [&](double sum, tuple<string, double, int> tu) {
				if (vData[row++][col])
					return sum + dampingFactor * get<1>(tu) / static_cast<double>(get<2>(tu));
				return sum;
				});
			col++;
			differences.push_back(abs(pre - get<1>(t)));
			});
		return accumulate(differences.begin(), differences.end(), 0.0);
	}
public:
	PageRank(const vector<string>&, const vector<vector<int>>&);
	void calculate();
	vector<tuple<string, double, int>> getPageRank() const;
};

// PageRank Parametrized Constructor 
PageRank::PageRank(const vector<string>& names, const vector<vector<int>>& data){
	int index{ 0 };
	this->error = 0.0001;
	this->dampingFactor = 0.85;
	this->vData = data;
	for_each(names.begin(), names.end(), [&](string str) {
		pageRanks.push_back(tuple<string, double, int>(str, 1.0, accumulate(vData[index].begin(), vData[index].end(), 0))); index++;
		});
}

// Calculate PageRank until sums error is less than 0.0001
void PageRank::calculate() {
	while (error < calcPRsums());
}

// PageRank Getter
vector<tuple<string, double, int>> PageRank::getPageRank() const {
	return this->pageRanks;
}

// Printing Class for PageRank
class PrintPageRank {
public:
	void Print(const vector<tuple<string, double, int>>&);
};

// Print function with parametrized tuple
void PrintPageRank::Print(const vector<tuple<string, double, int>>& tuples) {
	for (auto t : tuples)
		cout << setw(20) << setiosflags(ios::left) << get<0>(t) << "PageRank: " 
		<< setprecision(3) << setiosflags(ios::fixed) << get<1>(t) << endl;
}

int main() {
	cout << "Calculating PageRank from PageRank.html" << endl;
	Parser parser("PageRank.html");
	parser.parse();

	Graph graph;
	graph.build(parser.getvData());

	PageRank pagerank(graph.getNames(), graph.getGraph());
	pagerank.calculate();

	PrintPageRank print;
	print.Print(pagerank.getPageRank());

	return 0;
}