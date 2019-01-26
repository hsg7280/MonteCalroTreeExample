#include<iostream>
#include<cstdlib>
#include<ctime>

#define CNT 3
#define LEN 10

#define INIT_VAL 1
#define STRIKE 100
#define BALL 10

using namespace std;

//this is the game class for generating numbera and calculating score
class BaseballGame{
private :
	int answer[CNT];
public :
	// constructor and generate answer
	BaseballGame(){
		int i;
		srand(time(NULL));
		for(i=0; i<CNT; i++){
			answer[i] = rand()%10;
		}
	}
	
	// calculate score
	int getScore(int number[CNT]){
		int score = 0;
		int i, j;
		int tmp_answer[CNT];
		
		// this function is maked for prohibit double calulation
		for( i=0; i<CNT; i++ ){
			tmp_answer[i] = answer[i];
		}
		
		// correct number and position then add 10 point and remove number
		for( i=0; i<CNT; i++ ){
			if( number[i] == tmp_answer[i] ){
				score+=STRIKE;
				tmp_answer[i]=-1;
			}
		}
		
		// corrent number then add 1 point and remove number
		for( i=0; i<CNT; i++ ){
			for( j=0; j<CNT; j++ ){
				if( number[j] == tmp_answer[i] ){
					score+=BALL;
					tmp_answer[i]=-1;
					break;
				}
			}
		}
		
		return score;
	}
	
	// print answer
	void printAnswer(){
		int i;
		cout << "answer :";
		for( i=0; i<CNT; i++ ){
			cout << answer[i];
		}
		cout << endl;
	}
	
	friend class BaseballGame;
};

// Monte carlo tree node
class MonteCarloTreeNode{
private :
	int challenge;
	int success;
	int deep;
	MonteCarloTreeNode** child;
public : 
	// this is the constructor for making init value and child node
	MonteCarloTreeNode(int _deep){
		int i;
		
		// set initial value
		challenge=INIT_VAL;
		success=INIT_VAL;
		deep = _deep;
		
		// if this node is not leaf node then create child node.
		if( deep <= CNT){
			child = new MonteCarloTreeNode*[LEN];
			
			for(i=0; i<LEN; i++){
				child[i] = new MonteCarloTreeNode(deep+1);
			}
		}
		else{
			child = NULL;
		}
	}
	
	// select child node excluding removed node
	int selectChild(){
		int child_idx;
		int i;
		int success_sum;
		int success_sum_tmp;
 		int selected_value;
		
		// calulate total sum of child node
		success_sum=0;
		for(i=0; i<LEN; i++){
			success_sum += child[i]->success / child[i]->challenge;
		}
		
		// generate random value
		selected_value = rand() % success_sum;
		
		child_idx = 0;
		
		// choose child index using selected_value
		success_sum_tmp = 0;
		for(i=0; i<LEN && success_sum_tmp < selected_value; i++){
			if( child[i]->success != 0 ){
				success_sum_tmp += child[i]->success / child[i]->challenge;
				child_idx = i;
			}
		}
		
		return child_idx;
		
	}
	
	// update success and challenge value using traverse return value in MonteCarloTree
	void updateNumber(int number[CNT], int score){
		int i;
		int idx;
		MonteCarloTreeNode* current_node = this;
		
		for(i=0; i<CNT; i++){
			idx=number[i];
			current_node = current_node->child[idx];
			
			current_node->success += score;
			current_node->challenge += 1;
		}
	}
	
	// remove success value using traverse return value in MonteCarloTree
	void removeNumber(int number[CNT]){
		int i, j, k;
		MonteCarloTreeNode* main_node = this;
		MonteCarloTreeNode* current_node;
		
		for(i=0; i<LEN; i++){
			current_node = main_node->child[i];
			
			// clear number becuase that number is not anwser.
			for(j=0; j<CNT; j++){
				if( i==number[j] ){
					current_node->success = 0;
				}
			}
			
			if( current_node->deep <= CNT ){
				current_node->removeNumber(number);
			}
		}
	 
	}
	
	// remove success value using traverse return value in MonteCarloTree
	void updateExistNumber(int number[CNT]){
		int i, j, k;
		MonteCarloTreeNode* main_node = this;
		MonteCarloTreeNode* current_node;
		
		for(i=0; i<LEN; i++){
			current_node = main_node->child[i];
			
			// clear number becuase that number is not anwser.
			for(j=0; j<CNT; j++){
				if( i==number[j] ){
					current_node->success += BALL;
				}
			}
			
			if( current_node->deep <= CNT ){
				current_node->updateExistNumber(number);
			}
		}
	 
	}
	
	friend class MonteCarloTree;
};

// Monte carlo Tree
class MonteCarloTree{
private :
	MonteCarloTreeNode* root;
	BaseballGame game;
	
public :
	MonteCarloTree(){
		// create root node ( deep is 1)
		root = new MonteCarloTreeNode(1);
		
		// show answer
		game.printAnswer();
	}
	
	// traverse random number
	void traverse(){
		int i;
		int idx;
		int score;
		int number[CNT];
		MonteCarloTreeNode* current_node = root;
		
		// select random number
		for(i=0; i<CNT; i++){
			idx = current_node->selectChild();
			number[i] = idx;
			current_node = current_node->child[idx];
		}
		
		// get the score
		score = game.getScore(number);
		
		// if score value is bigger than 0 then update success and challenge value
		if( score > 0 ){
			root->updateNumber(number, score);
			
			// if all number is correct then update all exist number.
			if( score/STRIKE + (score/BALL) % BALL == CNT ){
				root->updateExistNumber(number);
			}
		}
		// if score value is 0 then remove number because that number is not answer
		else{
			root->removeNumber(number);
		}
		
	}
	
	// print best choice
	void printBestChoice(){
		int i, j;
		int value;
		int max_value;
		int idx;
		int number[CNT];
		MonteCarloTreeNode* main_node = root;
		MonteCarloTreeNode* current_node;
		
		for(i=0; i<CNT; i++){
			max_value=0;
			
			for(j=0; j<LEN; j++){
				current_node = main_node->child[j];
				
				value = current_node->success / current_node->challenge;
				if( max_value < value ){
					max_value = value;
					idx = j;
				}
			}
			
			main_node = main_node->child[idx];
			number[i] = idx;
		}
		
		cout << "best choice : {";
		for(i=0; i<CNT; i++){
			cout << number[i];
		}
		cout << "}" << endl;
	}
};

int main(){
	int i;
	
	MonteCarloTree tree;
	
	// traverse using random value
	for(i=0; i<100; i++){
		tree.traverse();
	}
	
	// print best choice in this status
	tree.printBestChoice();
	
	return 0;
}
