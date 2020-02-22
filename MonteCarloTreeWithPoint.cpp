#include<iostream>
#include<cstdlib>
#include<ctime>

#define CNT 3
#define LEN 10
#define INIT_CHALLENGE 1
#define INIT_SUCCESS 1

using namespace std;

//this is the game class for generating numbera and calculating score
class BaseballGame{
private :
	int* answer;
	int len;
public :
	// constructor and generate answer
	BaseballGame(int number_length){
		int i;
		
		len = number_length;
		// make a anwser
		srand(time(NULL));
		answer = new int(len);
		
		for(i=0; i<len; i++){
			*(answer+i) = rand()%10;
		}
	}
	
	// calculate score
	int getScore(int* number){
		int score = 0;
		int i, j;
		int *tmp_answer;
		int *tmp_number;
		tmp_answer = (int *)malloc(sizeof(int) * len);
		tmp_number = (int *)malloc(sizeof(int) * len);
		
		// this function is maked for prohibit double calulation
		for( i=0; i<len; i++ ){
			*(tmp_answer+i) = *(answer+i);
			*(tmp_number+i) = *(number+i);
			
		}
		
		// strike score(10 point) : correct number and position. if correct then remove number
		for( i=0; i<len; i++ ){
			if( *(tmp_number+i) == *(tmp_answer+i) ){
				score+=10;
				*(tmp_answer+i)=-1;
				*(tmp_number+i)=-1;
			}
		}
		
		// ball score(1 point) : corrent number and different position. if cooret then remove number
		for( i=0; i<len; i++ ){
			if( *(tmp_number+i)==-1 ){
				continue;
			}
			for( j=0; j<len; j++ ){
				if( *(tmp_answer+j) == -1 ){
					continue;
				}
				else if( *(tmp_number+i) == *(tmp_answer+j) ){
					score+=1;
					*(tmp_answer+j)=-1;
					*(tmp_number+i)=-1;
					break;
				}
			}
		}
		free(tmp_answer);
		free(tmp_number);
		
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
	MonteCarloTreeNode(int _deep, int max_deep){
		int i;
		
		// init challenge. you can fix it.
		challenge=INIT_CHALLENGE;
		// init success. you can modify it.
		success=INIT_SUCCESS;
		deep = _deep;
		
		// if this node is not leaf node then create child node.
		if( deep < max_deep){
			// if this node should make a child node
			child = new MonteCarloTreeNode*[LEN];
			for(i=0; i<LEN; i++){
				// make next node
				*(child+i) = new MonteCarloTreeNode(deep+1, max_deep);
			}
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
			success_sum += (*(child+i))->success / (*(child+i))->challenge;
		}
		
		// generate random value
		selected_value = rand() % success_sum;
		
		child_idx = 0;
		
		// choose child index using selected_value
		success_sum_tmp = 0;
		for(i=0; i<LEN && success_sum_tmp < selected_value; i++){
			if( (*(child+i))->success != 0 ){
				success_sum_tmp += (*(child+i))->success / (*(child+i))->challenge;
				child_idx=i;
			}
		}
		
		return child_idx;
		
	}
	
	// traverse if reached the end node then calculate score.
	int traverse(BaseballGame* game, int* select_number, int max_deep){
		int score;
		int selected_child;
		
		// not a leaf node
		if( deep < max_deep-1 ){
			selected_child = this->selectChild();
			*(select_number+deep)=selected_child;
			
			// recursive call
			score = (*(child+selected_child))->traverse(game, select_number, max_deep);
			
			(*(child+selected_child))->success += score;
			(*(child+selected_child))->challenge += 1;
		}
		// leaf node
		else if( deep == max_deep - 1 ){
			selected_child = this->selectChild();
			*(select_number+deep)=selected_child;
			
			//get score
			score = game->getScore(select_number);
			
			(*(child+selected_child))->success += score;
			(*(child+selected_child))->challenge += 1;
		}
		
		return score;
	}
	
	// remove value using traverse return value 0 in MonteCarloTree
	void removeNumber(int* number, int max_deep){
		int i;
		// not a leaf node
		if( deep < max_deep - 1 ){
			// reset number of current node
			for( i=0; i<CNT; i++ ){
				(*(child+*(number+i)))->success = 0;
				(*(child+*(number+i)))->challenge = 1;
			}
			for( i=0; i<LEN; i++ ){
				(*(child+i))->removeNumber(number, max_deep);
			}
		} 
		else if( deep == max_deep - 1 ){
			for( i=0; i<CNT; i++ ){
				(*(child+*(number+i)))->success = 0;
				(*(child+*(number+i)))->challenge = 1;
			}
		}
	}
	
	// remove value using traverse return value 0 in MonteCarloTree
	void removeOneNumber(int number, int max_deep){
		int i;
		// not a leaf node
		if( deep < max_deep - 1 ){
			// reset number of current node
			(*(child+number))->success = 0;
			(*(child+number))->challenge = 1;
			for( i=0; i<LEN; i++ ){
				(*(child+i))->removeOneNumber(number, max_deep);
			}
		} 
		else if( deep == max_deep - 1 ){
			(*(child+number))->success = 0;
			(*(child+number))->challenge = 1;
		}
	}
	
	// remove all value
	void setNumber(int* number, int max_deep){
		int i;
		// not a leaf node
		if( deep < max_deep - 1 ){
			// reset number of current node
			for( i=0; i<LEN; i++ ){
				if( i != *(number+deep) ){
					(*(child+i))->success = 0;
					(*(child+i))->challenge = 1;
				}
				(*(child+i))->setNumber(number, max_deep);
			}
		} 
		else if( deep == max_deep - 1 ){
			for( i=0; i<LEN; i++ ){
				if( i != *(number+deep) ){
					(*(child+i))->success = 0;
					(*(child+i))->challenge = 1;
				}
			}
		}
	}
	
	
	friend class MonteCarloTree;
};

// Monte carlo Tree
class MonteCarloTree{
private :
	MonteCarloTreeNode* root;
	int len;
	BaseballGame *game;
public :
	MonteCarloTree(BaseballGame* _game, int number_length){
		// create root node ( deep is 0)
		len = number_length;
		root = new MonteCarloTreeNode(0, len);
		game = _game;
	}
	
	// traverse random number
	void traverse(){
		int i, j;
		int *number = (int*) malloc(sizeof(int) * len);
		int is_exists;
		int score;
		
		// select traverse with 1st child
		score = root->traverse(game, number, len);
		
		if( score == 0 ){
			root->removeNumber(number, len);
		}
		else if( (score/10) + (score%10) == len ){
			// remove non-target number
			for( i=0; i<LEN; i++){
				is_exists = false;
				for(j=0; j<len; j++){
					// target number
					if( *(number+j) == i ){
						is_exists = true;
						break;
					}
				}
				// non-target number
				if( is_exists == false ){
					root->removeOneNumber(i, len);
				}
			}
		}
		else if( score == len * 10 ){
			// save score
			root->setNumber(number, len);
		}
		
		free(number);
		
	}
	
	// print best choice
	void printChoice(){
		int i;
		int *number = (int *)malloc(sizeof(int) * len);
		int score;
		
		score = root->traverse(game, number, len);
		
		cout << "choice : {";
		for(i=0; i<CNT; i++){
			cout << *(number+i) << endl;
		}
		cout << "}" << endl;
		
		free(number);
	}
};

int main(){
	int i;
	int* num;
	BaseballGame* game;
	
	game = new BaseballGame(CNT);
	game->printAnswer();
	
	MonteCarloTree tree(game, CNT);
	
	// traverse using random value
	for(i=0; i<100; i++){
		tree.traverse();
	}
	
	// print best choice in this status
	tree.printChoice();
	
	return 0;
}

