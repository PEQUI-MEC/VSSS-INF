#ifndef STRATEGY_HPP_
#define STRATEGY_HPP_
#define PI 3.14159265453
#include "opencv2/opencv.hpp"
#include "LS.cpp"
#include "../pack-capture-gui/capture-gui/Robot.hpp"
#include "SerialW.hpp"
#include <math.h>
#include "Ann.hpp"

#define PREDICAO_NUM_SAMPLES 15
// Parametros para atacante sem bola

#define ENTRADA_GOL_INVERSO	1

//Parametros para atacante com bola
#define AVOID_GOLEIRO_APOLO		0

// Parametros do Defensor com bola no ataque
#define DEF_RATIO			1.0f

// Parametros do Defensor na defesa
#define ZAGA_CENTRALIZADA			1





class Strategy
{
public:
	Ann * ann;
	bool useAnn_flag = false;

	Robot Goalkeeper;
	Robot Attack;
	Robot Defense;
	Robot Opponent;
	cv::Point Ball;
	cv::Point FutureBall;
	int distBall;
	LS LS_ball_x;
	LS LS_ball_y;
	cv::Point Ball_Est;
	cv::Point target = cv::Point(-1,-1);
	cv::Point targetlock = cv::Point(-1,-1);
	int height, width;
	int COMPRIMENTO_CAMPO_TOTAL;
	int COMPRIMENTO_PISTA;
	int LARGURA_CAMPO 	;
	int TAMANHO_GOL  	;
	int TAMANHO_AREA  	;
	bool GOAL_DANGER_ZONE = false;
	bool ADV_NA_AREA = false;
	int BANHEIRA		;
	int DIVISAO_AREAS	;
	int OFFSET_BANHEIRA	;
	int MEIO_GOL_X		;
	int MEIO_GOL_Y		;
	int MAX_GOL_Y		;
	int MIN_GOL_Y		;
	int LINHA_ZAGA		;
	int LIMITE_AREA_X	;
	// Parametros para atacante sem bola
	int OFFSET_RATIO	;
	int CONE_RATIO		;
	int COUNT_DECISION  ;
	int MAX_DECISION_COUNT;
	int TARGET_LOCK_COUNT;
	int MAX_TARGET_LOCK_COUNT;
	// Parametros do Defensor na defesa
	int DESLOCAMENTO_ZAGA_ATAQUE	;
	int BALL_RADIUS;

	Strategy()									// TUDO EM METROS
	{
		LS_ball_x.init(PREDICAO_NUM_SAMPLES,1);
		LS_ball_y.init(PREDICAO_NUM_SAMPLES,1);
	}

	void set_constants(int w, int h) {
		width = w;
		height = h;
		COMPRIMENTO_PISTA = 	round(1.50*float(width)/1.70); // ATENÇÃO !!!!! Valor sem referência, medida absoluto
		COMPRIMENTO_CAMPO_TOTAL = 	width; // Do fund de um gol ao fundo do outro gol

		LARGURA_CAMPO 	= 		height;
		TAMANHO_GOL 	= 		round(0.35*float(height)/1.30);
		TAMANHO_AREA 	= 		247;
		LIMITE_AREA_X	=		113;

		BANHEIRA		=	round((0.50*COMPRIMENTO_CAMPO_TOTAL))+round(0.16*float(width)/1.70);
		DIVISAO_AREAS	=	round((0.50*COMPRIMENTO_CAMPO_TOTAL) - 10); // O valor negativo é um offset para não ficar exatamente no meio.
		OFFSET_BANHEIRA=	round(0.20*float(width)/1.70);
		MEIO_GOL_X		=	round(COMPRIMENTO_CAMPO_TOTAL-round(0.05*float(width)/1.70));
		MEIO_GOL_Y		=	round(LARGURA_CAMPO/2);
		MAX_GOL_Y		=	round(MEIO_GOL_Y + TAMANHO_GOL/2);
		MIN_GOL_Y		=	round(MEIO_GOL_Y - TAMANHO_GOL/2);
		LINHA_ZAGA		=	round(0.30*COMPRIMENTO_CAMPO_TOTAL);

		// Parametros para atacante sem bola
		OFFSET_RATIO	=	round(0.12*float(width)/1.70);
		CONE_RATIO		=	round(0.8*float(width)/1.70);
		MAX_DECISION_COUNT = 10;
		COUNT_DECISION = MAX_DECISION_COUNT;
		TARGET_LOCK_COUNT = 10;
		MAX_TARGET_LOCK_COUNT = 10;
		// Parametros do Defensor na defesa
		DESLOCAMENTO_ZAGA_ATAQUE	=	round(1.3*float(width)/1.70);
		BALL_RADIUS = 100;
	}

	bool set_ann(const char * annName) {
		// bool pra fazer alguma verificação se deu certo (no futuro)
		ann = new Ann(annName);
		return true;
	}

	void set_ball_radius(int ballRadius)
	{
			BALL_RADIUS = ballRadius;
	}

	int get_ball_radius()
	{
		return BALL_RADIUS;
	}

	void set_area_limit_x(int areaLimitX)
	{
			LIMITE_AREA_X = areaLimitX;
	}

	int get_area_limit_x()
	{
		return LIMITE_AREA_X;
	}

	void set_areas_division(int areasDivision)
	{
			DIVISAO_AREAS = areasDivision;
	}

	int get_areas_division()
	{
		return DIVISAO_AREAS;
	}

	void set_goal_max(int goalMax)
	{
			MAX_GOL_Y = goalMax;
	}

	int get_goal_max()
	{
		return MAX_GOL_Y;
	}

	void set_goal_min(int goalMin)
	{
			MIN_GOL_Y = goalMin;
	}

	int get_goal_min()
	{
		return MIN_GOL_Y;
	}

	void set_defense_line(int defenseLine)
	{
			LINHA_ZAGA = defenseLine;
	}

	int get_defense_line()
	{
		return LINHA_ZAGA;
	}

	void set_banheira(int banheira)
	{
			BANHEIRA = banheira;
	}

	int get_banheira()
	{
		return BANHEIRA;
	}

	void set_goal_center(int x, int y)
	{
			MEIO_GOL_Y = y;
			MEIO_GOL_X = x;
	}

	int get_goal_center_x()
	{
		return MEIO_GOL_X;
	}

	int get_goal_center_y()
	{
		return MEIO_GOL_Y;
	}

	void get_targets(vector<Robot> * pRobots) {
		vector<Robot> robots = *pRobots;
		if(useAnn_flag) {
			double * inputs = (double *) calloc(ann->ninputs, sizeof(double)); // inicializa com zero para o caso de não identificar todos os robôs
			// preenche entradas da rede
			inputs[0] = Ball.x;
			inputs[1] = Ball.y;
			for(int i = 0, j = 0; i < robots.size() && i < 6; i++, j+=2) {
				inputs[j] = robots[i].position.x;
				inputs[j+1] = robots[i].position.y;
			}
			// executa a rede com as entradas. a saída está em ann.outputs
			ann->Execute(inputs);
			// atualiza o target nos robôs
			for(int i = 0, j = 0; i < 3 && j+1 < ann->noutputs; i++, j+=2) {
				robots[i].target.x = ann->outputs[j];
				robots[i].target.y = ann->outputs[j+1];
			}
		}
		else {
			// peguei esse código do camcap.hpp, é possível que algumas partes comentadas não funcionem por chamar funções da visão
			for(int i =0; i<3; i++) {
		    switch (robots[i].role)	{
		      case 0:
		      robots[i].target = get_gk_target();
		      robots[i].fixedPos = Goalkeeper.fixedPos;
		      if(GOAL_DANGER_ZONE) {
		        robots[i].histWipe();
		      }
		      break;
		      case 2:
		      robots[i].target = get_atk_target(robots[i].position, robots[i].orientation);
		      robots[i].fixedPos = Attack.fixedPos;
		      robots[i].status = Attack.status;
		      /*for(int j=0;j<vision->Adv_Main.size();j++){
		        if ( sqrt(pow(vision->Adv_Main[j].x - robots[i].position.x, 2) + pow(vision->Adv_Main[j].y - robots[i].position.y, 2)) < 50) {
		          robots[i].histWipe();
		        }
		      }*/
		      break;
		      case 1:
		      robots[i].target = get_def_target(robots[i].position);
		      robots[i].fixedPos = Defense.fixedPos;
		      robots[i].status = Defense.status;
		      /*for(int j=0;j<vision->Adv_Main.size();j++){
		          if ( sqrt(pow(vision->Adv_Main[j].x - robots[i].position.x, 2) + pow(vision->Adv_Main[j].y - interface.robot_list[i].position.y, 2)) < 50) {
		          interface.robot_list[i].spin = true;
		        }
		      }*/
		      break;
		      case 3:
		      robots[i].target = get_opp_target(robots[i].position, robots[i].orientation);
		      robots[i].fixedPos = Opponent.fixedPos;
		      robots[i].status = Opponent.status;
		      break;
		    } // switch
		    //cout<<robots[0].target.x<<" - "<<robots[0].target.y<<endl;
			}
		}

		// devolve o vetor de robots com as alterações
		*pRobots = robots;
	} // get_targets

	cv::Point get_atk_target(cv::Point robot, double orientation) { // Estratégia de ataque clássico (Antigo Ojuara)

		Attack.previous_status = Attack.status;

		distBall = sqrt(pow(robot.x - Ball.x, 2) + pow(robot.y - Ball.y, 2));
		//	cout<<"Status - "<<Attack.status<<endl;

		Attack.status = 0;


		//	cout<<"atan(1) "<<atan2(float(MIN_GOL_Y - robot.y),float(COMPRIMENTO_CAMPO - robot.x))<<" atan(2) "<<atan2(float(MAX_GOL_Y - robot.y) , float(COMPRIMENTO_CAMPO - robot.x))<<" orientation "<<orientation<<endl;
		if (Ball.x > DIVISAO_AREAS) { //Bola no ataque?
			Attack.fixedPos=false;
			//cout<<"Bola no Ataque "<<"|";
			if(distBall < round(0.08*float(width)/1.70) ) {
				//Posse de bola?????????? SIM

				//Definicao de acao para acelerar em direcao ao gol
				if (robot.y > MAX_GOL_Y && orientation >= 0) {
					orientation = orientation - double(PI);
				}
				if (robot.y < MIN_GOL_Y && orientation < 0) {
					orientation = orientation + double(PI);
				}

				if (robot.x >= COMPRIMENTO_CAMPO_TOTAL - LIMITE_AREA_X  && (robot.y < MIN_GOL_Y || robot.y > MAX_GOL_Y) ) {
					// Posse de Bola? Retangulo lateral?

					//cout<<"Retangulo Lateral - ";

					// robo esta nos retangulos laterais(com aprox 20cm a partir do fundo do gol) do gol adversario?

					target.x = Ball.x; //Vai na bola
					target.y = Ball.y;
					Attack.status = 3;
				}
				else if (Attack.previous_status == 3||COUNT_DECISION<MAX_DECISION_COUNT)
				{
					target.x = Ball.x;
					target.y = Ball.y;
					Attack.status = 4;
					COUNT_DECISION--;
					if(COUNT_DECISION<=0)
					COUNT_DECISION = MAX_DECISION_COUNT;

				}
				else if (orientation >  atan2(float(MIN_GOL_Y - robot.y),float((COMPRIMENTO_CAMPO_TOTAL-0.1*float(width)/1.70) - robot.x)) &&
				orientation <  atan2(float(MAX_GOL_Y - robot.y) , float((COMPRIMENTO_CAMPO_TOTAL - 0.1*float(width)/1.70) - robot.x)) && robot.x < Ball.x) {
					//Posse de bola? && Orientação robô + bola voltada para o lado adversário?

					//				cout<<"Angulos "<<endl;
					Attack.status = 2;

					//			cout<<"Status - "<<Attack.status<<endl;
					if ( robot.y < MEIO_GOL_Y) {
						target.x = MEIO_GOL_X;
						target.y = MAX_GOL_Y;
					}
					else {
						target.x = MEIO_GOL_X;
						target.y = MIN_GOL_Y;
					}

					if(TARGET_LOCK_COUNT==MAX_TARGET_LOCK_COUNT) {
						targetlock.x = target.x;
						targetlock.y = target.y;
						TARGET_LOCK_COUNT--;
					}
					if (TARGET_LOCK_COUNT < MAX_TARGET_LOCK_COUNT) {
						target.x = targetlock.x;
						target.y = targetlock.y;
						TARGET_LOCK_COUNT--;
						if (TARGET_LOCK_COUNT <= 0) {
							TARGET_LOCK_COUNT = MAX_TARGET_LOCK_COUNT;
						}
					}


				}
				else if ( robot.y < MAX_GOL_Y && robot.y > MIN_GOL_Y && robot.x < (COMPRIMENTO_CAMPO_TOTAL - LIMITE_AREA_X)) {

					Attack.status = 2;

					if ( robot.y < MEIO_GOL_Y) {
						target.x = MEIO_GOL_X;
						target.y = MAX_GOL_Y;
						//					cout<<"Centro 1 "<<endl;
					}
					else if (robot.y >= MEIO_GOL_Y) {
						target.x = MEIO_GOL_X;
						target.y = MIN_GOL_Y;
						//					cout<<"Centro 2"<<endl;
					}
					if(TARGET_LOCK_COUNT==MAX_TARGET_LOCK_COUNT) {
						targetlock.x = target.x;
						targetlock.y = target.y;
						TARGET_LOCK_COUNT--;
					}
					if (TARGET_LOCK_COUNT < MAX_TARGET_LOCK_COUNT) {
						target.x = targetlock.x;
						target.y = targetlock.y;
						TARGET_LOCK_COUNT--;
						if (TARGET_LOCK_COUNT <= 0) {
							TARGET_LOCK_COUNT = MAX_TARGET_LOCK_COUNT;
						}
					}



				}
				else if (Ball.x < robot.x)
				{
					target.x = robot.x + 50;
				}
				else {
					target.y = Ball.y;
					target.x = Ball.x;
				}


			}
			else { //Sem a bola
				//	cout<<"Sem Posse de Bola - ";
				if(robot.x > Ball_Est.x+round(0.04*float(width)/1.70)) { // Atacante a frente da bola?
					//	cout<<"Atk na frente da bola - ";
					target.x = Ball_Est.x-round(0.16*float(width)/1.70);	//Coisas fuzzy que ninguem entende a partir daqui----------------------------|

					// Ataque adiantado, da a volta na bola
					float diff = (float(Ball_Est.y) - float(robot.y))/float(LARGURA_CAMPO);

					float offset = pow(cos(PI*diff/2),6)*float(OFFSET_RATIO)*cos(2*PI*diff);

					float mixrb = abs(diff)*float(Ball_Est.y) + (1-abs(diff))*float(robot.y);



					if(diff>0)
					target.y = round(mixrb - offset);
					else
					target.y = round(mixrb + offset);

					// Não permite que o target seja colocado além dos limites do campo
					if(target.y<0)
					target.y = round(mixrb + offset);
					else if(target.y>LARGURA_CAMPO)
					target.y = round(mixrb - offset);


				}
				else {
					//				cout<<"Atk atras da bola - ";
					// Ataque recuado
					if(robot.x < COMPRIMENTO_CAMPO_TOTAL - LIMITE_AREA_X) {
						//					cout<<"Fora da area - ";

						if (distBall < BALL_RADIUS ) {
							Attack.status = 1;
							//					cout<<"Status - "<<Attack.status<<endl;
						}

						float phi = atan(float(MEIO_GOL_Y - Ball.y)/float(MEIO_GOL_X - Ball.x));		// Angulo entre o gol e a bola
						float theta = atan(float(MEIO_GOL_Y - robot.y)/float(MEIO_GOL_X - robot.x));	// Angulo entre o gol e o atacante
						target.x = Ball_Est.x - round(CONE_RATIO*cos(phi)*2*(abs(phi-theta))/PI);
						target.y = Ball_Est.y - round(CONE_RATIO*sin(phi)*2*(abs(phi-theta))/PI);
						if(target.y<0 || target.y > LARGURA_CAMPO) {
							//					cout<<"Alvo fora - ";
							target.x = Ball_Est.x;
							target.y = Ball_Est.y;
						}
					} else {
						//					cout<<"Na area - ";
						target.x = Ball_Est.x;
						target.y = Ball_Est.y;				// Fim das coisas fuzzy queninguem entende----------------------------|
					}
				}
			}
		}
		else {
			//cout<<"Bola na Defesa - ";
			Attack.fixedPos=true;
			//Bola na defesa
			target.x = BANHEIRA;
			if(Ball.y < LARGURA_CAMPO/2)
			target.y = Ball.y + OFFSET_BANHEIRA;
			else
			target.y = Ball.y - OFFSET_BANHEIRA;
			//		cout<<endl;
		}


		//cout<<target.x<<" - "<<target.y<<endl;
		//cout<<" - Attack Status - "<<Attack.status<<" Previous - "<<Attack.previous_status<<endl;
		return target;

	}

	cv::Point get_def_target(cv::Point robot) { // Estratégia de defesa clássica (Antigo Lenhador)
		distBall = round(sqrt(pow(robot.x - Ball.x, 2) + pow(robot.y - Ball.y, 2)));

		Defense.status = 0;

		if (Ball.x < DIVISAO_AREAS) { //Bola na defesa?
			Defense.fixedPos=false;
			//cout<<"Bola na Defesa - ";

			if (distBall < round(0.08*float(width)/1.70) && robot.x < Ball.x - 15) { //Posse de bola? && Orientação robô + bola voltada para o lado dversário?
				//cout<<"Posse de Bola "<<"|";

				Defense.status = 2;

				target.x = MEIO_GOL_X; //Vai na projeçao da bola na linha vertical do meio do gol
				target.y = Ball.y;
				if ( target.y > MAX_GOL_Y) target.y = MIN_GOL_Y;
				if ( target.y < MIN_GOL_Y) target.y = MAX_GOL_Y;
			}
			else { // Sem a bola
				//cout<<"Sem Posse de Bola - ";
				if (Ball.x < LIMITE_AREA_X&&(Ball.y < LARGURA_CAMPO/2-TAMANHO_AREA/2 || Ball.y > LARGURA_CAMPO/2+TAMANHO_AREA/2)) {
					//Bola na linha de fundo
					//	cout<<"Bola na linha de fundo - ";
					if(robot.x>Ball.x+17) {
						//Bola atras
						//cout<<"Bola atras - ";
						if(Ball.y<LARGURA_CAMPO/2-TAMANHO_AREA/2 - 17) {
							// lINHA DE FUNDO SUPERIOR

							if(robot.y>LARGURA_CAMPO/2-TAMANHO_AREA/2) {
								target.x = LIMITE_AREA_X;
							} else {
								target.x = Ball_Est.x;
							}

							target.y =  LARGURA_CAMPO/2-TAMANHO_AREA/2 - 17;
							//cout<<"Fundo superior - ";
						} else {
							// lINHA DE FUNDO INFERIOR
							if(robot.y<LARGURA_CAMPO/2+TAMANHO_AREA/2) {
								target.x = LIMITE_AREA_X;
							} else {
								target.x = Ball_Est.x;
							}

							target.y =  LARGURA_CAMPO/2+TAMANHO_AREA/2 + 17;
							//cout<<"Fundo inferior - ";
						}
					} else {
						//cout<<"Bola frente - ";
						//Bola frente/lado
						target.x = Ball_Est.x;
						target.y = Ball_Est.y;

					}

				} else if (robot.x > Ball.x + round(0.04*float(width)/1.70)) {
					target.x = Ball_Est.x-round(0.16*float(width)/1.70);	//Coisas fuzzy que ninguem entende a partir daqui----------------------------|
					//cout<<" Defesa adiantado, da a volta na bola";
					float diff = float(Ball_Est.y - robot.y)/float(LARGURA_CAMPO);
					float offset = pow(cos(PI*diff/2),6)*float(OFFSET_RATIO)*cos(2*PI*diff);
					float mixrb = abs(diff)*float(Ball_Est.y) + (1-abs(diff))*float(robot.y);

					if(diff>0)
					target.y = round(mixrb - offset);
					else
					target.y = round(mixrb + offset);

					// Não permite que o target seja colocado além dos limites do campo
					if(target.y<0)
					target.y = round(mixrb + offset);
					else if(target.y>LARGURA_CAMPO)
					target.y = round(mixrb - offset);

				}
				else {
					// Ataque recuado
					//   Cap.Bruno          if(robot.x < COMPRIMENTO_CAMPO-round(0.2*float(width)/1.70)) {
					//cout<<"Dentro linha area - ";
					// Dentro da area
					float phi = atan(float(MEIO_GOL_Y - Ball.y))/float(MEIO_GOL_X - Ball.x);		// Angulo entre o gol e a bola
					float theta = atan(float(MEIO_GOL_Y - robot.y))/float(MEIO_GOL_X - robot.x);	// Angulo entre o gol e o atacante
					target.x = Ball_Est.x - round(CONE_RATIO*cos(phi)*2*(abs(phi-theta))/PI);
					target.y = Ball_Est.y - round(CONE_RATIO*sin(phi)*2*(abs(phi-theta))/PI);
					if(target.y<0 || target.y > LARGURA_CAMPO) {
						target.x = Ball_Est.x;
						target.y = Ball_Est.y;
					}
					/*   Cap.Bruno          } else {
					//cout<<"fora linha area - ";
					target.x = Ball_Est.x;
					target.y = Ball_Est.y;				// Fim das coisas fuzzy que ninguem entende----------------------------|
				}  */
			}
		}


	}
	else {
		//		cout<<"Bola no Ataque - ";
		// Bola no ataque
		target.x = LINHA_ZAGA;
		Defense.fixedPos=true;
		if (ZAGA_CENTRALIZADA) {
			//cout<<"Entrou no if porra "<<endl;

			float alpha = float(Ball_Est.y - DIVISAO_AREAS)/float(DESLOCAMENTO_ZAGA_ATAQUE*COMPRIMENTO_CAMPO_TOTAL - DIVISAO_AREAS);
			//	cout<<"alpha  "<<long(alpha);
			target.y = round((alpha*LARGURA_CAMPO/2) + (1+alpha)*Ball_Est.y);
		}
		else {
			target.y = Ball_Est.y;
		}
	}

	if(target.x < LIMITE_AREA_X && (target.y > LARGURA_CAMPO/2-TAMANHO_AREA/2 && target.y < LARGURA_CAMPO/2+TAMANHO_AREA/2)) {
		target.x = LINHA_ZAGA;
		Defense.fixedPos=true;
		target.y = Ball.y;
		//	cout<<"Nao deixa area - ";
		//		Não permite que o alvo esteja dentro da área
	}




	//cout<<endl;
	return target;
}

cv::Point get_opp_target(cv::Point robot, double orientation){ // Estratégia de oponente clássico (Estratégia Clássica espelhada)

	Opponent.previous_status = Opponent.status;

	distBall = sqrt(pow(robot.x - Ball.x, 2) + pow(robot.y - Ball.y, 2));
	//	cout<<"Status - "<<Attack.status<<endl;

	Opponent.status = 0;


	//	cout<<"atan(1) "<<atan2(float(MIN_GOL_Y - robot.y),float(COMPRIMENTO_CAMPO - robot.x))<<" atan(2) "<<atan2(float(MAX_GOL_Y - robot.y) , float(COMPRIMENTO_CAMPO - robot.x))<<" orientation "<<orientation<<endl;
	if (Ball.x < DIVISAO_AREAS) { //Bola no ataque?
		Opponent.fixedPos=false;
		cout<<"Bola no Ataque "<<"|";
		if(distBall < round(0.08*float(width)/1.70) ) {
			cout<<"Posse de bola"<<"|";

			//Definicao de acao para acelerar em direcao ao gol
			if (robot.y > MAX_GOL_Y && orientation >= 0) {
				orientation = orientation - double(PI);
			}
			if (robot.y < MIN_GOL_Y && orientation < 0) {
				orientation = orientation + double(PI);
			}

			if (robot.x <= LIMITE_AREA_X  && (robot.y < MIN_GOL_Y || robot.y > MAX_GOL_Y) ) {
				// Posse de Bola? Retangulo lateral?

				cout<<"Retangulo Lateral"<<"|";

				// robo esta nos retangulos laterais(com aprox 20cm a partir do fundo do gol) do gol adversario?

				target.x = Ball.x; //Vai na bola
				target.y = Ball.y;
				Opponent.status = 3;
			}
			else if (Opponent.previous_status == 3||COUNT_DECISION<MAX_DECISION_COUNT)
			{
				target.x = Ball.x;
				target.y = Ball.y;
				Opponent.status = 4;
				COUNT_DECISION--;
				if(COUNT_DECISION<=0)
				COUNT_DECISION = MAX_DECISION_COUNT;

			}
			else if (orientation >  atan2(float(MIN_GOL_Y - robot.y),float(0-robot.x)) &&
			orientation <  atan2(float(MAX_GOL_Y - robot.y),float(0-robot.x)) &&
			robot.x > Ball.x) {
				//Posse de bola? && Orientação robô + bola voltada para o lado adversário?
				cout<<"Orientação Correta"<<"|";
				//				cout<<"Angulos "<<endl;
				Opponent.status = 2;

				//			cout<<"Status - "<<Attack.status<<endl;
				if ( robot.y < MEIO_GOL_Y) {
					target.x = 0;
					target.y = MAX_GOL_Y;
				}
				else {
					target.x = 0;
					target.y = MIN_GOL_Y;
				}

				if(TARGET_LOCK_COUNT==MAX_TARGET_LOCK_COUNT) {
					targetlock.x = target.x;
					targetlock.y = target.y;
					TARGET_LOCK_COUNT--;
				}
				if (TARGET_LOCK_COUNT < MAX_TARGET_LOCK_COUNT) {
					target.x = targetlock.x;
					target.y = targetlock.y;
					TARGET_LOCK_COUNT--;
					if (TARGET_LOCK_COUNT <= 0) {
						TARGET_LOCK_COUNT = MAX_TARGET_LOCK_COUNT;
					}
				}


			}
			else if ( robot.y < MAX_GOL_Y &&
				robot.y > MIN_GOL_Y &&
				robot.x > (0.1*float(width)/1.70)) {
					cout<<"Situação de Gol"<<"|";
					Opponent.status = 2;

					if ( robot.y < MEIO_GOL_Y) {
						target.x = 0;
						target.y = MAX_GOL_Y;
						//					cout<<"Centro 1 "<<endl;
					}
					else if (robot.y >= MEIO_GOL_Y) {
						target.x = 0;
						target.y = MIN_GOL_Y;
						//					cout<<"Centro 2"<<endl;
					}
					if(TARGET_LOCK_COUNT==MAX_TARGET_LOCK_COUNT) {
						targetlock.x = target.x;
						targetlock.y = target.y;
						TARGET_LOCK_COUNT--;
					}
					if (TARGET_LOCK_COUNT < MAX_TARGET_LOCK_COUNT) {
						target.x = targetlock.x;
						target.y = targetlock.y;
						TARGET_LOCK_COUNT--;
						if (TARGET_LOCK_COUNT <= 0) {
							TARGET_LOCK_COUNT = MAX_TARGET_LOCK_COUNT;
						}
					}



				}
				else if (Ball.x > robot.x)
				{
					target.x = robot.x - 50;
				}
				else {
					target.y = Ball.y;
					target.x = Ball.x;
				}


			}
			else { //Sem a bola
				cout<<"Sem Posse de Bola "<<"|";
				if(robot.x < Ball_Est.x-round(0.04*float(width)/1.70)) { // Atacante a frente da bola?
					cout<<"Atk na frente da bola"<<"|";
					target.x = Ball_Est.x+round(0.16*float(width)/1.70);	//Coisas fuzzy que ninguem entende a partir daqui----------------------------|

					// Ataque adiantado, da a volta na bola
					float diff = (float(Ball_Est.y) - float(robot.y))/float(LARGURA_CAMPO);

					float offset = pow(cos(PI*diff/2),6)*float(OFFSET_RATIO)*cos(2*PI*diff);

					float mixrb = abs(diff)*float(Ball_Est.y) + (1-abs(diff))*float(robot.y);



					if(diff>0)
					target.y = round(mixrb - offset);
					else
					target.y = round(mixrb + offset);

					// Não permite que o target seja colocado além dos limites do campo
					if(target.y<0)
					target.y = round(mixrb + offset);
					else if(target.y>LARGURA_CAMPO)
					target.y = round(mixrb - offset);


				}
				else {
					cout<<"Atk atras da bola"<<"|";
					// Ataque recuado
					if(robot.x >  round(0.2*float(width)/1.70)) {
						cout<<"Fora da area"<<"|";

						if (distBall < 100 ) {
							Opponent.status = 1;
							//					cout<<"Status - "<<Attack.status<<endl;
						}

						float phi = atan(float(MEIO_GOL_Y - Ball.y)/float(0-Ball.x));		// Angulo entre o gol e a bola
						float theta = atan(float(MEIO_GOL_Y - robot.y)/float(0-robot.x));	// Angulo entre o gol e o atacante
						target.x = Ball_Est.x + round(CONE_RATIO*cos(phi)*2*(abs(phi-theta))/PI);
						target.y = Ball_Est.y + round(CONE_RATIO*sin(phi)*2*(abs(phi-theta))/PI);
						if(target.y<0 || target.y > LARGURA_CAMPO) {
							//					cout<<"Alvo fora - ";
							target.x = Ball_Est.x;
							target.y = Ball_Est.y;
						}
					} else {
						cout<<"Na area"<<"|";
						target.x = Ball_Est.x;
						target.y = Ball_Est.y;				// Fim das coisas fuzzy queninguem entende----------------------------|
					}
				}
			}
			cout<<endl;
		}
		else { //cout<<"Bola na Defesa - ";
		distBall = round(sqrt(pow(robot.x - Ball.x, 2) + pow(robot.y - Ball.y, 2)));

		Opponent.status = 0;
		Opponent.fixedPos=false;


		if (distBall < round(0.08*float(width)/1.70) && robot.x < Ball.x - 15) { //Posse de bola? && Orientação robô + bola voltada para o lado dversário?
			//cout<<"Posse de Bola "<<"|";

			Opponent.status = 2;

			target.x = 0; //Vai na projeçao da bola na linha vertical do meio do gol
			target.y = Ball.y;
			if ( target.y > MAX_GOL_Y) target.y = MIN_GOL_Y;
			if ( target.y < MIN_GOL_Y) target.y = MAX_GOL_Y;
		}
		else { // Sem a bola
			//cout<<"Sem Posse de Bola - ";
			if ((Ball.x > COMPRIMENTO_CAMPO_TOTAL-LIMITE_AREA_X)&&(Ball.y < LARGURA_CAMPO/2-TAMANHO_AREA/2 || Ball.y > LARGURA_CAMPO/2+TAMANHO_AREA/2)) {
				//Bola na linha de fundo
				//	cout<<"Bola na linha de fundo - ";
				if(robot.x<Ball.x-17) {
					//Bola atras
					//cout<<"Bola atras - ";
					if(Ball.y<LARGURA_CAMPO/2-TAMANHO_AREA/2 - 17) {
						// lINHA DE FUNDO SUPERIOR

						if(robot.y>LARGURA_CAMPO/2-TAMANHO_AREA/2) {
							target.x = COMPRIMENTO_CAMPO_TOTAL-LIMITE_AREA_X;
						} else {
							target.x = Ball_Est.x;
						}

						target.y =  LARGURA_CAMPO/2-TAMANHO_AREA/2 - 17;
						//cout<<"Fundo superior - ";
					} else {
						// lINHA DE FUNDO INFERIOR
						if(robot.y<LARGURA_CAMPO/2+TAMANHO_AREA/2) {
							target.x = COMPRIMENTO_CAMPO_TOTAL-LIMITE_AREA_X;
						} else {
							target.x = Ball_Est.x;
						}

						target.y =  LARGURA_CAMPO/2+TAMANHO_AREA/2 + 17;
						//cout<<"Fundo inferior - ";
					}
				} else {
					//cout<<"Bola frente - ";
					//Bola frente/lado
					target.x = Ball_Est.x;
					target.y = Ball_Est.y;

				}

			} else if (robot.x < Ball.x - round(0.04*float(width)/1.70)) {
				target.x = Ball_Est.x+round(0.16*float(width)/1.70);	//Coisas fuzzy que ninguem entende a partir daqui----------------------------|
				//cout<<" Defesa adiantado, da a volta na bola";
				float diff = float(Ball_Est.y - robot.y)/float(LARGURA_CAMPO);
				float offset = pow(cos(PI*diff/2),6)*float(OFFSET_RATIO)*cos(2*PI*diff);
				float mixrb = abs(diff)*float(Ball_Est.y) + (1-abs(diff))*float(robot.y);

				if(diff>0)
				target.y = round(mixrb - offset);
				else
				target.y = round(mixrb + offset);

				// Não permite que o target seja colocado além dos limites do campo
				if(target.y<0)
				target.y = round(mixrb + offset);
				else if(target.y>LARGURA_CAMPO)
				target.y = round(mixrb - offset);

			}
			else {
				// Ataque recuado
				//cout<<"Robo atras - ";
				//   Cap.Bruno          if(robot.x < COMPRIMENTO_CAMPO-round(0.2*float(width)/1.70)) {
				//cout<<"Dentro linha area - ";
				// Dentro da area
				float phi = atan(float(MEIO_GOL_Y - Ball.y))/float(0 - Ball.x);		// Angulo entre o gol e a bola
				float theta = atan(float(MEIO_GOL_Y - robot.y))/float(0 - robot.x);	// Angulo entre o gol e o atacante
				target.x = Ball_Est.x + round(CONE_RATIO*cos(phi)*2*(abs(phi-theta))/PI);
				target.y = Ball_Est.y + round(CONE_RATIO*sin(phi)*2*(abs(phi-theta))/PI);
				if(target.y<0 || target.y > LARGURA_CAMPO) {
					target.x = Ball_Est.x;
					target.y = Ball_Est.y;
				}
				/*   Cap.Bruno          } else {
				//cout<<"fora linha area - ";
				target.x = Ball_Est.x;
				target.y = Ball_Est.y;				// Fim das coisas fuzzy que ninguem entende----------------------------|
			}  */
		}
	}


}


/*if(target.x >COMPRIMENTO_CAMPO-LIMITE_AREA_X && (target.y > LARGURA_CAMPO/2-TAMANHO_AREA/2 && target.y < LARGURA_CAMPO/2+TAMANHO_AREA/2)) {
target.x = COMPRIMENTO_CAMPO-LINHA_ZAGA;
Opponent.fixedPos=true;
target.y = Ball.y;
//	cout<<"Nao deixa area - ";
//		Não permite que o alvo esteja dentro da área
} */

return target;

}



cv::Point get_Ball_Est() {
	return Ball_Est;
}
void set_Ball_Est(cv::Point b) {
	Ball_Est =  b;
}
void set_Ball(cv::Point b) {
	Ball = b;
	LS_ball_x.addValue(Ball.x);
	LS_ball_y.addValue(Ball.y);

	Ball_Est.x =  LS_ball_x.estimate(5);
	Ball_Est.y =  LS_ball_y.estimate(5);

}


cv::Point get_gk_target() {
	Goalkeeper.fixedPos=true;
	Goalkeeper.target.x = 60;
	if (Ball.x < DIVISAO_AREAS)
	{   // Bola na defesa

		Goalkeeper.target.y = Ball_Est.y;
		// ir na projeção da bola na linha de fundo

		if(Ball.x<LINHA_ZAGA)
		GOAL_DANGER_ZONE = true;
		else
		GOAL_DANGER_ZONE = false;

		if (Goalkeeper.target.y > MAX_GOL_Y)
		Goalkeeper.target.y = MAX_GOL_Y;
		else if (Goalkeeper.target.y < MIN_GOL_Y)
		Goalkeeper.target.y = MIN_GOL_Y;




		/*			if( Ball.x<LIMITE_AREA_X && Ball.y>LARGURA_CAMPO/2-TAMANHO_AREA/2 && Ball.y<LARGURA_CAMPO/2+TAMANHO_AREA/2 )
		{
		//cout<<"BOLA NA AREA"<<endl;
		ADV_NA_AREA=false;
		for(int i=0;i<Adv_Main.size();i++){
		if((Adv_Main[i].x<LIMITE_AREA_X&&Adv_Main[i].y>LARGURA_CAMPO/2-TAMANHO_AREA/2&&Adv_Main[i].y<LARGURA_CAMPO/2+TAMANHO_AREA/2))
		{
		//cout<<"Adv NA AREA"<<endl;
		ADV_NA_AREA=true;
	}
}

if(!ADV_NA_AREA)
{
Goalkeeper.target=Ball;
}
}*/
if (Ball.x<LIMITE_AREA_X && (Ball.y<LARGURA_CAMPO/2-TAMANHO_AREA/2 || Ball.y>LARGURA_CAMPO/2+TAMANHO_AREA/2))
{

	//cout<<"bola nas laterais da area"<<endl;

	if (Ball.y > LARGURA_CAMPO/2)
	Goalkeeper.target.y = 354;
	else if (Ball.y < LARGURA_CAMPO/2)
	Goalkeeper.target.y = 135;
}
}
else
{   // Bola no ataque
	Goalkeeper.target.y = Ball_Est.y;
	// ir na projeção da bola na linha de fundo

	if (Goalkeeper.target.y > MAX_GOL_Y)
	Goalkeeper.target.y = MAX_GOL_Y;
	else if (Goalkeeper.target.y < MIN_GOL_Y)
	Goalkeeper.target.y = MIN_GOL_Y;

}
//std::cout<<"GK TARGET "<<Goalkeeper.target.x<<" - "<<Goalkeeper.target.y<<std::endl;
return Goalkeeper.target;

}

};
#endif /* STRATEGY_HPP_ */
