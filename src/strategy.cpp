#include "strategy.hpp"
#include <cmath>
#include <utility>

#define PI 3.14159265453
#define PREDICAO_NUM_SAMPLES 15

// role
#define GOALKEEPER 0
#define DEFENDER 1
#define ATTACKER 2
// #define OPPONENT 3

// state
#define NORMAL_STATE 0
#define CORNER_STATE 1
#define	STEP_BACK 2
// #define	ADVANCING_STATE 3
#define	TRANSITION_STATE 4
#define	SIDEWAYS 5
#define DEF_CORNER_STATE 6
#define ATK_PENALTI_STATE 7
#define INVIABLE_TRANSITION_STATE 8
#define BALL_IN_AREA_STATE 9

// velocidade
#define MAX_VEL 1.4f
#define MEDIAN_VEL 0.8f
#define SLOW_VEL 0.35f

using namespace CONST;

Strategy::Strategy() : distBall(0) {
	LS_ball_x.init(PREDICAO_NUM_SAMPLES,1);
	LS_ball_y.init(PREDICAO_NUM_SAMPLES,1);
}

void Strategy::set_constants(int w, int h) {
	width = w;
    height = h;

	// GOAL_DANGER_ZONE = false,
	// ADV_NA_AREA = false;

    // ABS_PLAYING_FIELD_WIDTH = (int) round(1.50*float(width)/1.70);
    ABS_GOAL_TO_GOAL_WIDTH = width;
    ABS_ROBOT_SIZE = (int) round(0.08*float(width)/1.70);
    // ABS_BALL_SIZE = (int) round(0.04*float(width)/1.70);
    COORD_MID_FIELD_X = ABS_GOAL_TO_GOAL_WIDTH/2;

    ABS_FIELD_HEIGHT = height;
    ABS_GOAL_SIZE_Y = (int) round(0.40*float(height)/1.30);
    ABS_GOAL_SIZE_X = (int) round(0.10*float(width)/1.70); /**< tamanho do gol em X */
    COORD_GOAL_DEF_FRONT_X = ABS_GOAL_SIZE_X;
    COORD_GOAL_ATK_FRONT_X = ABS_GOAL_TO_GOAL_WIDTH - ABS_GOAL_SIZE_X;
    COORD_GOAL_MID_Y = ABS_FIELD_HEIGHT/2;
    COORD_GOAL_UP_Y = ABS_FIELD_HEIGHT/2 - ABS_GOAL_SIZE_Y/2;
    COORD_GOAL_DWN_Y = ABS_FIELD_HEIGHT/2 + ABS_GOAL_SIZE_Y/2;

    ABS_BOX_SIZE_Y = (int) round(0.70*float(height)/1.30); /**< tamanho da área em Y */
    ABS_BOX_SIZE_X = (int) round(0.15*float(width)/1.70); /**< tamanho da área em X */
    COORD_BOX_DEF_X = ABS_GOAL_SIZE_X + ABS_BOX_SIZE_X; // linha da frente da área
    COORD_BOX_ATK_X = ABS_GOAL_TO_GOAL_WIDTH - (ABS_GOAL_SIZE_X + ABS_BOX_SIZE_X); // linha da frente da área adversária
    COORD_BOX_UP_Y = (int) round((ABS_FIELD_HEIGHT - ABS_BOX_SIZE_Y)/2.0); // canto de cima do gol
    COORD_BOX_DWN_Y = (int) round(ABS_BOX_SIZE_Y + (ABS_FIELD_HEIGHT - ABS_BOX_SIZE_Y)/2.0); // canto de baixo do gol

    // variáveis ajustáveis
    corner_atk_limit = COORD_BOX_ATK_X - ABS_ROBOT_SIZE;
    def_corner_line = COORD_BOX_DEF_X;
    def_line = ABS_GOAL_TO_GOAL_WIDTH/3;
    // possession_distance = ABS_ROBOT_SIZE;
    collision_radius = ABS_ROBOT_SIZE/2;
    fixed_pos_distance = ABS_ROBOT_SIZE;
    max_approach = ABS_ROBOT_SIZE*2;
    max_collision_count = 30;
    goalie_line = COORD_GOAL_DEF_FRONT_X + ABS_ROBOT_SIZE/2;
    goalie_offset = ABS_ROBOT_SIZE;
    // transition_back_radius = ABS_ROBOT_SIZE*3;
    transition_y_distance = ABS_ROBOT_SIZE*4;

    // // Não usando
    BANHEIRA		=	(int) (round((0.50*COMPRIMENTO_CAMPO_TOTAL))+round(0.16*float(width)/1.70));
    // DIVISAO_AREAS	=	(int) round((0.50*COMPRIMENTO_CAMPO_TOTAL) - 10); // O valor negativo é um offset para não ficar exatamente no meio.
    // OFFSET_BANHEIRA	=	(int) round(0.20*float(width)/1.70);
    // MEIO_GOL_X		=	(int) round(COMPRIMENTO_CAMPO_TOTAL-round(0.05*float(width)/1.70));
    // MEIO_GOL_Y		=	(int) round(LARGURA_CAMPO/2);
    // MAX_GOL_Y		=	(int) round(MEIO_GOL_Y + TAMANHO_GOL/2);
    // MIN_GOL_Y		=	(int) round(MEIO_GOL_Y - TAMANHO_GOL/2);
    // LINHA_ZAGA		=	(int) round(0.30*COMPRIMENTO_CAMPO_TOTAL);

    // // Parametros para atacante sem bola
    // OFFSET_RATIO	=	(int) round(0.12*float(width)/1.70);
    // CONE_RATIO		=	(int) round(0.8*float(width)/1.70);
    // MAX_DECISION_COUNT = 10;
    // COUNT_DECISION = MAX_DECISION_COUNT;
    // TARGET_LOCK_COUNT = 10;
    // MAX_TARGET_LOCK_COUNT = 10;
    // // Parametros do Defensor na defesa
    // DESLOCAMENTO_ZAGA_ATAQUE	=	(int) round(1.3*float(width)/1.70);
	// BALL_RADIUS = 100;

}

void Strategy::get_targets(std::vector<cv::Point> advRobots) {
	adv = std::move(advRobots);

	get_variables(); // pega a linha em que o goleiro deve ficar da interface

	reset_flags();

	set_roles();

	gk_routine(gk);
	def_routine(def);
	atk_routine(atk);

	for(int i = 0; i < N_ROBOTS; i++) {
		if(!Robots::get_usingPotField(i)) position_to_vector(i);

		fixed_position_check(i); // faz a desaceleração e a parada dos robôs
		collision_check(i);
	}
} // get_targets

void Strategy::reset_flags() {
	for(int i = 0; i < Robots::SIZE; i++) {
		Robots::set_cmdType(i, Robots::CMD::VECTOR);
		Robots::set_fixedPos(i, false);
		Robots::set_usingPotField(i, false);
		if(transitions_enabled)
			atk = def = gk = 0;
	}
}

bool compare_pos(std::pair<int, std::pair<double, double> > a, std::pair<int, std::pair<double, double> > b) {
    return a.second.first < b.second.first;
}
void Strategy::set_roles() {
	if(!transitions_enabled) {
		for(int i = 0; i < Robots::SIZE; i++) {
			switch(Robots::get_role(i)) {
				case ATTACKER:
					atk = i;
				break;
				case DEFENDER:
					def = i;
				break;
				case GOALKEEPER:
					gk = i;
				break;
                default:break;
            }
		}
		return;
	}

    //< lista ordenada de melhores posicionamentos; id do robo -> <distancia do alvo, angulo com o objetivo>
    std::vector< std::pair<int, std::pair<double, double> > > bestAtk, bestGk;
    cv::Point goal = cv::Point(ABS_GOAL_SIZE_X, COORD_GOAL_MID_Y);

    // ----- SELEÇÃO DE CANDIDATOS
	// pega os candidatos a atacante e ordena por melhor posicionamento e angulo da bola com o gol
    // !TODO calcular os angulos de todos os (robôs -> bola, bola-> gol)
    // !TODO verificar se está mais perto do ponto da defesa do que defensor
    for(unsigned short i = 0; i < Robots::SIZE; i++) {
        std::pair<int, std::pair<double, double> > temp = {i, {distance(Robots::get_position(i), Ball), 0}};
        bestAtk.push_back(temp);
    }
    sort(bestAtk.begin(), bestAtk.end(), compare_pos); // !TODO pode ser que exista uma forma mais eficiente de ordenar isso

    // pega os candidatos a goleiro e ordena por melhor posicionamento e angulo para nosso gol
    // !TODO deve verificar depois se o goleiro pode sair do gol ou não
    for(unsigned short i = 0; i < Robots::SIZE; i++) {
        if(i == bestAtk.at(0).first) continue; // !TODO tirar isso se quiser que o atacante possa ser goleiro também
        std::pair<int, std::pair<double, double> > temp = {i, {distance(Robots::get_position(i), goal), 0}};
        bestGk.push_back(temp);
    }
    sort(bestGk.begin(), bestGk.end(), compare_pos);

    // ----- ATRIBUIÇÃO INICIAL DE PAPÉIS
    // a gente tem garantido que o vetor não é vazio pois a visão, no pior caso, nos dá (-1, -1)
    atk = bestAtk.at(0).first;
    gk = bestGk.at(0).first;
	// seta o robô restante pra defensor
	// não precisa inicializar pois é sempre setado no for
	for(int i = 0; i < Robots::SIZE; i++) {
		if(i == atk || i == gk) continue;
		def = i;
		break;
	}

	// ----- SITUAÇÕES ESPECÍFICAS (TROCAS)
    // situação de cruzamento
    if(Ball.x > corner_atk_limit && Ball.y > COORD_GOAL_UP_Y + ABS_ROBOT_SIZE && Ball.y < COORD_GOAL_DWN_Y - ABS_ROBOT_SIZE // se a bola tá dentro da área
         && distance(Robots::get_position(atk), Ball) > 2.0 * ABS_ROBOT_SIZE // atacante não tem a posse de bola
		 && Robots::get_position(def).x > COORD_MID_FIELD_X) { // defensor está depois do meio de campo esperando rebote
		int tmp = atk;
		atk = def;
		def = tmp;
    }
    // atacante perdeu a bola, é melhor defensor ir em cima
    else if(distance(Ball, Robots::get_position(atk)) > 3*ABS_ROBOT_SIZE // bola está longe do atacante
			 && Ball.x < Robots::get_position(atk).x // bola está atrás do atacante
			 && distance(Ball, Robots::get_position(def)) < COORD_GOAL_MID_Y // defensor está a até um meio campo de distância
			 && Ball.x > Robots::get_position(def).x + ABS_ROBOT_SIZE/2) { // e tá na frente do defensor
        int tmp = atk;
        atk = def;
        def = tmp;
    }

	// seta o atacante
	set_role(atk, ATTACKER);
	// seta o goleiro
	set_role(gk, GOALKEEPER);
	// seta o defensor
	set_role(def, DEFENDER);
}

void Strategy::set_role(int i, int role) {
	Robots::set_status(i, NORMAL_STATE);
	Robots::set_role(i, role);
	Robots::set_velocity(i, role);
}

bool Strategy::has_ball(int i) { // se i tem a bola, i pode levá-la pra frente
	double theta_i_ball = atan2(Ball.y - Robots::get_position(i).y, Ball.x - Robots::get_position(i).x);
	return distance(Robots::get_position(i), Ball) < ABS_ROBOT_SIZE*1.5 //< robô está perto da bola
		   && (abs(Robots::get_orientation(i) - theta_i_ball)*180/PI < 30 || abs(Robots::get_orientation(i) - theta_i_ball)*180/PI > 150) //< bola está em uma das frentes
		   && Ball.x > Robots::get_position(i).x + ABS_ROBOT_SIZE/2; // bola está na frente do atacante em relação ao gol inimigo
    ;
}

/*
void Strategy::overmind() {
	cv::Point goal = cv::Point(COORD_GOAL_ATK_FRONT_X, COORD_GOAL_MID_Y);

	double m1 = double(goal.y - Ball.y)/double(goal.x - Ball.x);
	double m2 = double(robots[atk].position.y - Ball.y)/double(robots[atk].position.x - Ball.x);
	// double ball_goal = distance(Ball, goal);
	// double r1 = ball_goal + max_approach;

	// double mup = double(COORD_GOAL_UP_Y - Ball.y)/double(COORD_GOAL_ATK_FRONT_X - Ball.x);
	// double mdwn = double(COORD_GOAL_DWN_Y - Ball.y)/double(COORD_GOAL_ATK_FRONT_X - Ball.x);
	// double theta = atan((mup-mdwn)/(1+mup*mdwn));

	double phi = atan((m2-m1)/(1+m2*m1));

	// para o defensor também
	double ballGoal = double(goal.y - Ball_Est.y)/double(goal.x - Ball_Est.x);
	double defBall = double(robots[def].position.y - Ball_Est.y)/double(robots[def].position.x - Ball_Est.x);
	double phiDef = atan((defBall-ballGoal)/(1+defBall*ballGoal));
	// cout << phiDef*180/PI << " phiDef" <<endl;

	if(robots[atk].cmdType == SPEED || robots[atk].status == CORNER_STATE) atk_mindcontrol = false;

	if(atk_mindcontrol) {
		robots[atk].cmdType = VECTOR;
		// robots[atk].target = goal;
		// if(abs(robots[atk].orientation) > 90) robots[atk].transAngle = 180;
		// else robots[atk].transAngle = 0;
		robots[atk].transAngle = lock_angle;
		robots[atk].vmax = 1.4;
		timeout++;

		if(timeout >= 60 || distance(robots[atk].position, Ball) > ABS_ROBOT_SIZE*2) {
			timeout = 0;
			atk_mindcontrol = false;
            robots[atk].vmax = default_vel[ATTACKER];
		}
	}

	if((distance(robots[atk].position, Ball) < ABS_ROBOT_SIZE*2) // robo ta perto da bola
 		&& (abs(phi)*180/PI < 20) && (Ball.x > robots[atk].position.x ) && // angulo (robo->bola, bola-> gol) é bom e a bola tá na frente do atk
		((((robots[atk].orientation - atan(m1))*180/PI) < 20 && ((robots[atk].orientation - atan(m1))*180/PI) > -20) || // bola ta em uma das frentes
		(((robots[atk].orientation - atan(m1))*180/PI) < -165 || ((robots[atk].orientation - atan(m1))*180/PI) > 165) ) // bola tá em uma das frentes
		// 	(robots[atk].orientation > atan(mdwn) && robots[atk].orientation < atan(mup))
		)
	{
		if(!atk_mindcontrol)
			lock_angle = atan2(double(robots[atk].position.y - Ball.y), - double(robots[atk].position.x - Ball.x));

		timeout = 0;
		atk_mindcontrol = true;
		// cout << "mindcontrol" << endl;
	}

	// Defender Overmind-------------------------
	if(def_mindcontrol) {
		cv::Point v = cv::Point(Ball.x - Ball_Est.x, Ball.y - Ball_Est.y);
		double module = sqrt(pow(v.x,2) + pow(v.y,2));
		double approach = distance(Ball, Ball_Est) * (distance(Ball, robots[atk].position)/(COORD_MID_FIELD_X + COORD_MID_FIELD_X/4));
		robots[atk].target.x = static_cast<int>(Ball.x - double(v.x/module) * approach);
		robots[atk].target.y = static_cast<int>(Ball.y - double(v.y/module) * approach);
		// cout << "approach " << approach << " proporção de distâncias " << distance(Ball, robots[atk].position)/(COORD_MID_FIELD_X + COORD_MID_FIELD_X/4) << endl;
		robots[atk].cmdType = VECTOR;
		position_to_vector(atk);
		robots[atk].vmax = 1.4;
		timeout++;

		if(timeout >= 30 || Ball.x < robots[atk].position.x) {
			timeout = 0;
			def_mindcontrol = false;
            robots[atk].vmax = default_vel[robots[atk].role];
		}
	}

	// Goalkeeper overmind-------------------
	if(!full_transition_enabled &&
		// (Ball.x < COORD_BOX_DEF_X && Ball.y > COORD_BOX_UP_Y && Ball.y < COORD_BOX_DWN_Y) &&
		(robots[atk].position.x < COORD_BOX_DEF_X + ABS_ROBOT_SIZE/2 &&
		robots[atk].position.y < COORD_BOX_DWN_Y + ABS_ROBOT_SIZE/2 &&
		robots[atk].position.y > COORD_BOX_UP_Y - ABS_ROBOT_SIZE/2) ) {

		robots[gk].fixedPos = true;
		// cout << " don't " << endl;
		// !TODO Olhar a necessidade de ser realizado cast para INT
		robots[gk].target = cv::Point((COORD_BOX_DEF_X + ABS_ROBOT_SIZE*1.5), COORD_GOAL_MID_Y);
		position_to_vector(gk);
		fixed_position_check(gk);
	}
}
*/

/*
void Strategy::set_flags() {
	danger_zone_1 = false;
	danger_zone_2 = false;
	half_transition = false;
	full_transition = false;

	if(Ball.x > COORD_MID_FIELD_X/3) {
		full_transition_enabled = true;
		half_transition_enabled = true;
	} else half_transition_enabled = false;

	if(Ball.x < robots[atk].position.x - ABS_ROBOT_SIZE*2 && Ball.x > robots[def].position.x) {
		danger_zone_1 = true;
	} else if (Ball.x < robots[atk].position.x && Ball.x < robots[def].position.x) {
		danger_zone_2 = true;
	}

}*/

void Strategy::fixed_position_check(int i) { // Para posição fixa
	if(Robots::get_fixedPos(i) && (Robots::get_cmdType(i) == Robots::CMD::VECTOR || Robots::get_cmdType(i) == Robots::CMD::POSITION)) {
        Robots::set_velocity(i, float(Robots::get_default_velocity(i) * pow(distance(Robots::get_position(i), Robots::get_target(i))/(ABS_GOAL_TO_GOAL_WIDTH/4.0), 2)));
		// cout << "vmax " << robots[i].vmax << " distancia "<< distance(Robots::get_position(i), Robots::get_target(i)) << " distancia max " << ABS_GOAL_TO_GOAL_WIDTH/4<<endl;

		if(Robots::get_velocity(i) > Robots::get_default_velocity(i)) Robots::set_velocity(i);
		if(Robots::get_velocity(i) < 0.3) Robots::set_velocity(i, 0.3f);

		if(distance(Robots::get_target(i), Robots::get_position(i)) <= fixed_pos_distance) {
			Robots::stop(i);
		}
	}
}

/*
bool Strategy::offensive_adv() {
	bool isDangerous = false;
	for (const auto &i : adv) {
		// se o adversário mais próximo da bola está a pelo menos duas vezes a distância de nosso gk a bola, não é perigoso
		if(distance(i, Ball) < distance(robots[gk].position, Ball) * 2) {
			isDangerous = true;
			break;
		}
	}

	return isDangerous;
}
*/

void Strategy::collision_check(int i) {
    if(Robots::get_fixedPos(i)) return;

    // se é atacante e está dentro do gol
    if (Robots::get_role(i) == ATTACKER &&
            Robots::get_position(i).y > COORD_GOAL_UP_Y && Robots::get_position(i).y < COORD_GOAL_DWN_Y &&
            Robots::get_position(i).x < ABS_GOAL_SIZE_X + 0.75 * ABS_ROBOT_SIZE) {

        // verifica se está colidindo
        if(distance(Robots::get_position(i), past_position[i]) <= collision_radius) {
            collision_count[i]++;
        } else {
            get_past(i);
            collision_count[i] = 0;
        }

        if(collision_count[i] >= max_collision_count) {
            cv::Point upper_corner;
            cv::Point lower_corner;
            double tempAngle;

            upper_corner.x = lower_corner.x = ABS_GOAL_SIZE_X;
            upper_corner.y = COORD_GOAL_UP_Y;
            lower_corner.y = COORD_GOAL_DWN_Y;

            if(distance(Robots::get_position(i), upper_corner) > distance(Robots::get_position(i), lower_corner) && Ball.y > COORD_GOAL_MID_Y){
                tempAngle = atan2(sin(past_transangle[i]+(PI/2)),cos(past_transangle[i]+(PI/2)));
            } else if (distance(Robots::get_position(i), upper_corner) < distance(Robots::get_position(i), lower_corner) && Ball.y < COORD_GOAL_MID_Y) {
                tempAngle = atan2(sin(past_transangle[i]-(PI / 2)), cos(past_transangle[i]-(PI / 2)));
            } else {
                tempAngle = Robots::get_transAngle(i);
            }

            Robots::set_target(i, cv::Point(Ball.x, Robots::get_position(i).y));
            Robots::set_command(i, tempAngle);
        }
    }
    else if(Robots::get_status(i) != CORNER_STATE && Robots::get_status(i) != ATK_PENALTI_STATE) {
		if(distance(Robots::get_position(i), past_position[i]) <= collision_radius &&
			distance(Robots::get_position(i), Robots::get_target(i)) > fixed_pos_distance) {
			collision_count[i]++;
		} else {
			get_past(i);
			collision_count[i] = 0;
		}

		if(collision_count[i] >= max_collision_count) {
			Robots::set_transAngle(i, atan2(sin(past_transangle[i]+PI),cos(past_transangle[i]+PI)));
		}
	}
}

/*
bool Strategy::cock_blocked() {
	bool at_least_one_blocked = false;

	// decrementa o timer se tiver
	if(transition_overmind_timeout > 0) {
		transition_overmind_timeout--;
		return false;
	}

	// verifica se os robôs estão colidindo e incrementa o tempo
	for(int i = 0; i < robots.size(); i++) {
		for(int j = i + 1; j < robots.size(); j++) {
			if(distance(Robots::get_position(i), robots[j].position) < ABS_ROBOT_SIZE * 1.5) {
				at_least_one_blocked = true;
			}
		}
	}

	if(!at_least_one_blocked) frames_blocked = 0;
	else {
		frames_blocked++;
		if(frames_blocked > 15) {
			transition_overmind_timeout = 60; // dois segundos
			return true;
		}
	}

	return false;
}
*/

bool Strategy::is_near(int i, cv::Point point) {
	return distance(Robots::get_position(i), point) < fixed_pos_distance;
}

void Strategy::position_to_vector(int i) {
	if(Robots::get_cmdType(i) != Robots::CMD::VECTOR)
		Robots::set_transAngle(i, atan2(double(Robots::get_position(i).y - Robots::get_target(i).y), - double(Robots::get_position(i).x - Robots::get_target(i).x)));
}

double Strategy::potField(int robot_index, cv::Point goal, int behavior) {
	Robots::set_usingPotField(robot_index, true);

	double gain_rep = 10;
	double gain_att = 1;
	double rep_radius = 0.2;
	double ball_rep_radius = 0.3;
	// float rot_angle = 0;

	ball_angle = 0;
	ball_mag = 0;
	pot_thetaX = 0;
	pot_thetaY = 0;
	pot_theta = 0;
	pot_goalTheta = 0;
	pot_goalMag = 0;

	for (unsigned short i = 0; i < adv.size(); i++) {
		if(distance_meters(Robots::get_position(robot_index), adv.at(i)) > 0 && distance_meters(Robots::get_position(robot_index), adv.at(i)) < rep_radius) {
			pot_magnitude[i] = gain_rep*pow(1/(distance_meters(Robots::get_position(robot_index), adv.at(i)) - 1/rep_radius), 2)/2;
			pot_angle[i] = atan2(double(Robots::get_position(robot_index).y - adv.at(i).y),- double(Robots::get_position(robot_index).x - adv.at(i).x));

			if(pot_rotation_decision(robot_index,goal,adv.at(i)) == 0) {
				pot_magnitude[i] = 0;
			} else if(pot_rotation_decision(robot_index,goal,adv.at(i))>0) {
					pot_angle[i] = atan2(sin(pot_angle[i]+PI/2),cos(pot_angle[i]+PI/2));
					// printf("HORARIO ");
			} else {
				pot_angle[i] = atan2(sin(pot_angle[i]+3*PI/2),cos(pot_angle[i]+3*PI/2));
					// printf("ANTI-HORARIO ");
			}
			// printf("adv[%d] x %d, y %d| magnitude [%d] %f | angle[%d] %f\n",i,adv.at(i).x,adv.at(i).y,i,pot_magnitude[i],i,pot_angle[i]*180/PI);
		} else {
			pot_magnitude[i] = 0;
			pot_angle[i] = 0;
		}
	}

	int j = 3;

	for (int i = 0; i < 3; i++) {
		if(i != robot_index) {
			if(distance_meters(Robots::get_position(robot_index), Robots::get_position(i)) < rep_radius) {
				pot_magnitude[j] = gain_rep*pow(1/(distance_meters(Robots::get_position(robot_index), Robots::get_position(i)) - 1/rep_radius), 2)/2;
				pot_angle[j] = atan2(double(Robots::get_position(robot_index).y - Robots::get_position(i).y),- double(Robots::get_position(robot_index).x - Robots::get_position(i).x));
				if(pot_rotation_decision(robot_index,goal,Robots::get_position(i)) == 0) {
					pot_magnitude[j] = 0;
				} else if(pot_rotation_decision(robot_index,goal,Robots::get_position(i)) > 0) {
					pot_angle[j] = atan2(sin(pot_angle[j]+PI/2),cos(pot_angle[j]+PI/2));
					//	printf("HORARIO ");
				} else{
					pot_angle[j] = atan2(sin(pot_angle[j]+3*PI/2),cos(pot_angle[j]+3*PI/2));
					//	printf("ANTI-HORARIO ");
				}
			// printf("robots[%d] x %d, y %d|magnitude [%d] %f | angle[%d] %f\n",i,Robots::get_position(i).x,Robots::get_position(i).y,j,pot_magnitude[j],j,pot_angle[j]*180/PI);
			}else {
				pot_magnitude[j] = 0;
				pot_angle[j] = 0;
			}
			j++;
		}
	}

	switch(behavior) {
		case BALL_IS_OBS:
			if(distance_meters(Robots::get_position(robot_index), Ball) > 0 && distance_meters(Robots::get_position(robot_index), Ball) < ball_rep_radius) {

				// !TODO Mudar variável para DOUBLE ou fazer cast para FLOAT?
				ball_mag = (gain_rep*pow(1/(distance_meters(Robots::get_position(robot_index), Ball) - 1/(ball_rep_radius)), 2)/2 );

				// !TODO Mudar variável para DOUBLE ou fazer cast para FLOAT?
				ball_angle = atan2(double(Robots::get_position(robot_index).y - Ball.y),- double(Robots::get_position(robot_index).x - Ball.x));
				if(pot_rotation_decision(robot_index,goal,Ball) == 0) {
					ball_mag = 0;
				} else if(pot_rotation_decision(robot_index,goal,Ball)>0) {
					// !TODO Mudar variável para DOUBLE ou fazer cast para FLOAT?
					ball_angle = atan2(sin(ball_angle+PI/2),cos(ball_angle+PI/2));
				//	printf("HORARIO ");
				} else {
					// !TODO Mudar variável para DOUBLE ou fazer cast para FLOAT?
					ball_angle = atan2(sin(ball_angle+3*PI/2),cos(ball_angle+3*PI/2));
					//	printf("ANTI-HORARIO ");
				}

				pot_thetaY += ball_mag*sin(ball_angle);
				pot_thetaX += ball_mag*cos(ball_angle);
			} else {
				ball_mag = 0;
				ball_angle = 0;
			}
			break;


		case BALL_ONLY_OBS:
			for(double &i : pot_magnitude) i = 0;

			if(distance_meters(Robots::get_position(robot_index), Ball) > 0 && distance_meters(Robots::get_position(robot_index), Ball) < ball_rep_radius) {
				// !TODO Mudar variável para DOUBLE ou fazer cast para FLOAT?
				ball_mag = (gain_rep*pow(1/(distance_meters(Robots::get_position(robot_index), Ball) - 1/ball_rep_radius), 2)/2.0);

				// !TODO Mudar variável para DOUBLE ou fazer cast para FLOAT?
				ball_angle = atan2(double(Robots::get_position(robot_index).y - Ball.y),- double(Robots::get_position(robot_index).x - Ball.x));
				if(pot_rotation_decision(robot_index,goal,Ball) == 0) {
					ball_mag = 0;
				} else if(pot_rotation_decision(robot_index,goal,Ball) > 0) {
					// !TODO Mudar variável para DOUBLE ou fazer cast para FLOAT?
					ball_angle = atan2(sin(ball_angle+PI/2),cos(ball_angle+PI/2));
						//	printf("HORARIO ");
				} else {
					// !TODO Mudar variável para DOUBLE ou fazer cast para FLOAT?
					ball_angle = atan2(sin(ball_angle+3*PI/2),cos(ball_angle+3*PI/2));
					//	printf("ANTI-HORARIO ");
				}
				pot_thetaY = ball_mag*sin(ball_angle);
				pot_thetaX = ball_mag*cos(ball_angle);
			} else {
				ball_mag = 0;
				ball_angle = 0;
			}
			break;

		case NO_OBS:
			for(double &i : pot_magnitude) i = 0;
			break;

		case BALL_NOT_OBS:
			break;
        default:break;
    }

	for (int i = 0; i < 5; i++) {
		pot_thetaY += pot_magnitude[i] * sin(pot_angle[i]);
		pot_thetaX += pot_magnitude[i] * cos(pot_angle[i]);
	}

	pot_goalTheta = atan2(double(Robots::get_position(robot_index).y -goal.y ),- double(Robots::get_position(robot_index).x - goal.x ));
	pot_goalMag = gain_att * pow(distance_meters(Robots::get_position(robot_index), goal), 2)/2;
	//	printf("goalMag %f | goalTheta %f\n",pot_goalMag,pot_goalTheta*180/PI);

	pot_thetaY += pot_goalMag * sin(pot_goalTheta);
	pot_thetaX += pot_goalMag * cos(pot_goalTheta);
	pot_theta = atan2(pot_thetaY, pot_thetaX);
	// pot_theta = robots[robot_index].orientation - pot_theta;
	// pot_theta = atan2(-sin(robots[robot_index].orientation - pot_theta),cos(robots[robot_index].orientation - pot_theta));
	// printf("THETA %f \n",pot_theta*180/PI);
	return pot_theta;
}

int Strategy::pot_rotation_decision(int robot_index,cv::Point goal, cv::Point obst) {
	int HORARIO = 1;
	int ANTI_HORARIO = -1;
	bool goal_ahead = Robots::get_position(robot_index).x > obst.x && obst.x > goal.x;
	bool goal_behind = Robots::get_position(robot_index).x < obst.x && obst.x < goal.x;

	if(obst.y < ABS_ROBOT_SIZE*2) {
		if(goal_ahead) return HORARIO;
		if(goal_behind) return ANTI_HORARIO;
	} else if(obst.y > (ABS_FIELD_HEIGHT - ABS_ROBOT_SIZE*2)) {
		if(goal_ahead) return ANTI_HORARIO;
		if(goal_behind) return HORARIO;
	}

	double angle_Obst_Robot = atan2(double(obst.y - Robots::get_position(robot_index).y), - double(obst.x - Robots::get_position(robot_index).x));
	double angle_Obst_Goal = atan2(double(obst.y - goal.y), - double(obst.x - goal.x));

	// printf("%f + %f = %f \n",fabs(angle_Obst_Robot)*180/PI,fabs(angle_Obst_Goal)*180/PI,(fabs(angle_Obst_Robot)+fabs(angle_Obst_Goal))*180/PI);
	if(angle_Obst_Robot > 0 && angle_Obst_Robot <= PI/2) { // ROBO NO PRIMEIRO QUAD
		if(angle_Obst_Goal > 0 && angle_Obst_Goal <= PI/2) { // OBJETIVO NO PRIMEIRO QUAD
			return 0;

		} else if(angle_Obst_Goal > PI/2 && angle_Obst_Goal <= PI) { // OBJETIVO NO SEGUNDO QUAD
			return ANTI_HORARIO;
		} else if(angle_Obst_Goal > -PI && angle_Obst_Goal <= -PI/2) {// OBJETIVO NO TERCEIRO QUAD
			return HORARIO;
		} else { // OBJETIVO NO QUARTO QUAD
			// printf("DECIDE" );
			if(fabs(angle_Obst_Robot)+fabs(angle_Obst_Goal) < PI) return HORARIO;
			else return ANTI_HORARIO;
		}
	} else if(angle_Obst_Robot > PI/2 && angle_Obst_Robot <= PI) { // ROBO NO SEGUNDO QUAD
		if(angle_Obst_Goal > 0 && angle_Obst_Goal <= PI/2) { // OBJETIVO NO PRIMEIRO QUAD
			return HORARIO;

		} else if(angle_Obst_Goal > PI/2 && angle_Obst_Goal <= PI) { // OBJETIVO NO SEGUNDO QUAD
			return 0;

		} else if(angle_Obst_Goal > -PI && angle_Obst_Goal <= -PI/2) {// OBJETIVO NO TERCEIRO QUAD
			return ANTI_HORARIO;

		} else { // OBJETIVO NO QUARTO QUAD
			// printf("DECIDE" );
			if(fabs(angle_Obst_Robot)+fabs(angle_Obst_Goal) < PI) return HORARIO;
			else return ANTI_HORARIO;
		}
	} else if(angle_Obst_Robot > -PI && angle_Obst_Robot <= -PI/2) {// ROBO NO TERCEIRO QUAD
		if(angle_Obst_Goal > 0 && angle_Obst_Goal <= PI/2) { // OBJETIVO NO PRIMEIRO QUAD
		//	printf("DECIDE" );
			if(fabs(angle_Obst_Robot)+fabs(angle_Obst_Goal) < PI) return ANTI_HORARIO;
			else return HORARIO;
		} else if(angle_Obst_Goal > PI/2 && angle_Obst_Goal <= PI) { // OBJETIVO NO SEGUNDO QUAD
			return HORARIO;
		} else if(angle_Obst_Goal > -PI && angle_Obst_Goal <= -PI/2) {// OBJETIVO NO TERCEIRO QUAD
			return 0;
		} else { // OBJETIVO NO QUARTO QUAD
			return ANTI_HORARIO;
		}
	} else { // ROBO NO QUARTO QUAD
		if(angle_Obst_Goal > 0 && angle_Obst_Goal <= PI/2) { // OBJETIVO NO PRIMEIRO QUAD
			return ANTI_HORARIO;
		} else if(angle_Obst_Goal > PI/2 && angle_Obst_Goal <= PI) { // OBJETIVO NO SEGUNDO QUAD
		//	printf("DECIDE" );
			if(fabs(angle_Obst_Robot)+fabs(angle_Obst_Goal) < PI) return ANTI_HORARIO;
			else return HORARIO;
		} else if(angle_Obst_Goal > -PI && angle_Obst_Goal <= -PI/2) {// OBJETIVO NO TERCEIRO QUAD
			return HORARIO;
		} else { // OBJETIVO NO QUARTO QUAD
			return 0;
		}
	}
}

void Strategy::def_wait(int i) {
    Robots::set_fixedPos(i, true);
	if(distance(Robots::get_position(i), Robots::get_target(i)) <= fixed_pos_distance) {
	    Robots::look_at(i, Ball);
	}
}

void Strategy::pot_field_around(int i) {
    Robots::set_usingPotField(i, true);

	cv::Point targets_temp;
	cv::Point goal = cv::Point(COORD_GOAL_ATK_FRONT_X, COORD_GOAL_MID_Y);

	double m1 = double(goal.y - Ball_Est.y)/double(goal.x - Ball_Est.x);
	double m2 = double(Robots::get_position(i).y - Ball_Est.y)/double(Robots::get_position(i).x - Ball_Est.x);
	// double ball_goal = distance(Ball_Est, goal);
	// double r1 = ball_goal + max_approach;

	double dist_up = distance(Ball_Est, cv::Point(COORD_GOAL_ATK_FRONT_X, COORD_GOAL_UP_Y));
	double dist_dwn = distance(Ball_Est, cv::Point(COORD_GOAL_ATK_FRONT_X, COORD_GOAL_DWN_Y));

	double beta = acos((pow(dist_up,2)+pow(dist_dwn,2)-pow(ABS_GOAL_SIZE_Y,2)) / (2*dist_up*dist_dwn));

	cv::Point v = cv::Point(goal.x - Ball_Est.x, goal.y - Ball_Est.y);
	double module = sqrt(pow(v.x,2) + pow(v.y,2));

	double phi = atan((m2-m1) / (1+m2*m1));
	// cout << "phi -> " << phi*180/PI << " 180 - phi -> " << 180 - phi*180/PI << endl;

	targets_temp.x = static_cast<int>(Ball_Est.x - (v.x/module) * max_approach);
	targets_temp.y = static_cast<int>(Ball_Est.y - (v.y/module) * max_approach);

	// crop
	if(targets_temp.y < 0) targets_temp.y = 0;
	if(targets_temp.y > ABS_FIELD_HEIGHT) targets_temp.y = ABS_FIELD_HEIGHT;
	if(targets_temp.x < 0) targets_temp.x = 0;
	if(targets_temp.x > COORD_GOAL_ATK_FRONT_X) targets_temp.x = COORD_GOAL_ATK_FRONT_X;

	Robots::set_target(i, targets_temp);
	Robots::set_transAngle(i, potField(i, targets_temp, BALL_ONLY_OBS));

	if(Robots::get_position(i).x > Ball.x + ABS_ROBOT_SIZE)
		action1 = false;

	if(is_near(i, targets_temp) || abs(phi) < beta/2 || action1) {
		v = cv::Point(Robots::get_position(i).x - Ball.x, Robots::get_position(i).y - Ball.y);
		// module = sqrt(pow(v.x,2) + pow(v.y,2));
		// targets_temp.x = (Ball.x - (v.x/module) * max_approach);
		// targets_temp.y = (Ball.y - (v.y/module) * max_approach);
		Robots::set_transAngle(i, atan2(double(Robots::get_position(i).y - Ball.y), - double(Robots::get_position(i).x - Ball.x)));
		action1 = true;
	}
}

void Strategy::crop_targets(int i) {
    cv::Point target = Robots::get_target(i);

	if(target.y < 0)
        target.y = 0;

	if(target.y > ABS_FIELD_HEIGHT)
		target.y = ABS_FIELD_HEIGHT;

	if(target.x < 0)
		target.x = 0;

	if(target.x > COORD_GOAL_ATK_FRONT_X)
		target.x = COORD_GOAL_ATK_FRONT_X;

	if(target.x < COORD_BOX_DEF_X && target.y > COORD_BOX_UP_Y && target.y < COORD_GOAL_DWN_Y) // não deixa entrar na área
		target.x = def_corner_line;

	Robots::set_target(i, target);
}

void Strategy::atk_routine(int i) {
	cv::Point target;
	cv::Point goal = cv::Point(COORD_GOAL_ATK_FRONT_X, COORD_GOAL_MID_Y);

	double m1 = double(goal.y - Ball.y)/double(goal.x - Ball.x);
	double m2 = double(Robots::get_position(i).y - Ball.y)/double(Robots::get_position(i).x - Ball.x);
	double phi = atan((m2-m1)/(1+m2*m1));

	Robots::set_velocity(i);

	if (has_ball(i)) {
		if (abs(phi) * 180/PI < 30) { //se tem ângulo bom com o gol, ELE CHUTA!
			Robots::set_velocity(i,MAX_VEL);
			Robots::set_status(i, ATK_PENALTI_STATE);
		} else {
			Robots::set_velocity(i, MEDIAN_VEL);
            //!TODO Ver como implementar posicionamento do robô atrás da bola
			//Robots::position(i,Ball,goal);
		}
	} else if (Ball.x > Robots::get_position(i).x && Ball.x > Robots::get_position(def).x) { //Bola na frente do atacante e do defensor
        Robots::set_velocity(i, MAX_VEL);
        //!TODO Ver como implementar posicionamento do robô atrás da bola NESTE CASO
        //Robots::position(i,Ball,goal);
	} else if (Ball.x < Robots::get_position(i).x && Ball.x > Robots::get_position(def).x) { //Bola entre o atacante e o defensor
			Robots::set_velocity(i,SLOW_VEL);
			//Robots::set_position(i, cv::Point(BANHEIRA, COORD_GOAL_UP_Y));
			Robots::set_target(i, cv::Point(BANHEIRA, COORD_GOAL_UP_Y));

	} else if(Ball.x < Robots::get_position(i).x && Ball.x < Robots::get_position(def).x) { //Bola atrás do atacante e do defensor
		if(Ball.y < COORD_GOAL_MID_Y){
			Robots::set_velocity(i,MAX_VEL);
			Robots::set_target(i, cv::Point(def_line, COORD_GOAL_UP_Y));	//volta pra parte de cima do gol
		}else{
			Robots::set_velocity(i,MAX_VEL);
			Robots::set_target(i, cv::Point(def_line, COORD_GOAL_DWN_Y));	//volta pra parte de baixo do gol
		}
	}

	//cod antigo daqui pra baixo

	if(Robots::get_status(i) != ATK_PENALTI_STATE) {
		if((Robots::get_position(def).x < COORD_MID_FIELD_X // se o defensor tá atrás do meio de campo
			&& Ball.x > corner_atk_limit + 5*ABS_ROBOT_SIZE  // se a bola tá depois do x do penalti
			&& Ball.y > COORD_GOAL_UP_Y && Ball.y < COORD_GOAL_DWN_Y) // se a bola tá no meio em y, isso é um penalti
			||
			(has_ball(i) && // atacante tem a posse de bola
			abs(phi)*180/PI < 30)) { // angulo com o gol tá bom
			Robots::set_status(i, ATK_PENALTI_STATE); // chuta
		}
		else if(Ball.x > corner_atk_limit && // se a bola tá depois da linha x da área adversária
			(Ball.y > COORD_GOAL_DWN_Y || Ball.y < COORD_GOAL_UP_Y)) { // e tá fora da área
			Robots::set_status(i, CORNER_STATE); // a bola tá no corner, gira
		}
		else if(Ball.x < def_corner_line) { // se a bola tá antes da nossa linha x da área
			Robots::set_status(i, DEF_CORNER_STATE); // a bola tá no nosso corner
		}
		else if(Ball.y > ABS_FIELD_HEIGHT - ABS_ROBOT_SIZE*1.5 || // se a bola tá na lateral de baixo
			Ball.y < ABS_ROBOT_SIZE*1.5) { // ou na lateral de cima
			Robots::set_status(i, SIDEWAYS); // sideways
		} else {
			Robots::set_status(i, NORMAL_STATE); // senão, age normalmente
		}
	} 
	

	switch(Robots::get_status(i)) {
		case NORMAL_STATE:
			// leva a bola pro gol deles
			Robots::set_cmdType(i, Robots::CMD::VECTOR);
			pot_field_around(i);

			break;

		case SIDEWAYS:

			Robots::set_cmdType(i, Robots::CMD::POSITION);

			// empurra a bola no canto mesmo
			if(Ball.y > ABS_FIELD_HEIGHT - ABS_ROBOT_SIZE*1.5) { // se a bola tá na lateral inferior
				if(distance(Robots::get_position(i), Ball) > ABS_ROBOT_SIZE // se a bola tá longe do robô
					|| Ball.x >= Robots::get_position(i).x + ABS_ROBOT_SIZE/1.5) { // ou se a bola não tá travada por ele
				    target = Ball;
				}
				else { // se a bola tá travada
					Robots::spin_counter_clockwise(i, 0.8);
				}

				if(Ball.x < Robots::get_position(i).x) { // se a bola tá atrás do atacante
					target.x = Ball.x - max_approach;
					target.y = int(Ball.y - max_approach/2.0);
				}
			}
			else if(Ball.y < ABS_ROBOT_SIZE*1.5) { // se a bola tá na lateral superior
				if(distance(Robots::get_position(i), Ball) > ABS_ROBOT_SIZE // se a bola tá longe do robô
					|| Ball.x >= Robots::get_position(i).x + ABS_ROBOT_SIZE/1.5) { // ou se a bola não tá travada
                    target = Ball;
				}
				else { // se a bola tá travada
					Robots::spin_clockwise(i, 0.8);
				}

				if(Ball.x < Robots::get_position(i).x) { // se a bola tá atrás do atacante
					target.x = Ball.x - max_approach;
					target.y = int(Ball.y + max_approach/2.0);
				}
			}

			Robots::set_target(i, target);

			break;

		case CORNER_STATE:

			Robots::set_cmdType(i, Robots::CMD::POSITION);

			if(Ball.y < COORD_GOAL_MID_Y) { // se a bola tá acima do gol
				if(Ball.y < COORD_GOAL_UP_Y) { // acima ou abaixo da trave, escolher o giro para levar a bola para o gol ou para faze-la entrar
					if(Ball.x > Robots::get_position(i).x && distance(Ball, Robots::get_position(i)) < ABS_ROBOT_SIZE)
						Robots::spin_clockwise(i, 0.8); // giro que leva a bola ao gol
					else
						target = Ball; // faz a bola travar no canto pra girar certo
				} else {
					if(Ball.y > Robots::get_position(i).y && distance(Ball, Robots::get_position(i)) < ABS_ROBOT_SIZE)
						Robots::spin_counter_clockwise(i, 0.8); // giro para faze-la entrar
					else
						target = Ball;
				}
			} else {
				if(Ball.y > COORD_GOAL_DWN_Y) { // se a bola tá abaixo do gol
					if(Ball.x > Robots::get_position(i).x && distance(Ball, Robots::get_position(i)) < ABS_ROBOT_SIZE)
						Robots::spin_counter_clockwise(i, 0.8);
					else
						target = Ball; // faz a bola travar no canto pra girar certo
				} else {
					if(Ball.y > Robots::get_position(i).y && distance(Ball, Robots::get_position(i)) < ABS_ROBOT_SIZE)
						Robots::spin_clockwise(i, 0.8); // giro para faze-la entrar
					else
						target = Ball;
				}
			}

			Robots::set_target(i, target);

			break;

		case DEF_CORNER_STATE:

			Robots::set_cmdType(i, Robots::CMD::POSITION);
		    //Robots::set_fixedPos(i, true);

			if(Ball.y > COORD_GOAL_MID_Y) {
				target.x = goalie_line;
				target.y = COORD_BOX_DWN_Y + ABS_ROBOT_SIZE;

				if(distance(Ball, Robots::get_position(i)) < ABS_ROBOT_SIZE && Ball.y > Robots::get_position(i).y)
					Robots::spin_counter_clockwise(i, 0.8);
				// if(distance(Robots::get_position(i), robots[i].target) <= fixed_pos_distance) robots[i].target = Ball;
				if(is_near(i, target) && Ball.y > Robots::get_position(i).y) {
					target = Ball;
				}

				Robots::set_target(i, target);
			} else {
				target.x = goalie_line;
				target.y = COORD_BOX_UP_Y - ABS_ROBOT_SIZE;

				if(distance(Ball, Robots::get_position(i)) < ABS_ROBOT_SIZE && Ball.y < Robots::get_position(i).y)
					Robots::spin_clockwise(i, 0.8);
				// if(distance(Robots::get_position(i), Robots::get_target(i)) <= fixed_pos_distance) robots[i].target = Ball;
				if(is_near(i, target) && Ball.y < Robots::get_position(i).y) {
					target = Ball;
				}

				Robots::set_target(i, target);
			}

			break;

		case ATK_PENALTI_STATE:

			Robots::set_cmdType(i, Robots::CMD::VECTOR);
            Robots::kick(i, goal);

			if(!has_ball(i))
				Robots::set_status(i, NORMAL_STATE);
	}
	//crop_targets(i);
}

void Strategy::def_routine(int i) {
	cv::Point target;
	cv::Point goal = cv::Point(COORD_GOAL_ATK_FRONT_X, COORD_GOAL_MID_Y);

	double m1 = double(goal.y - Ball_Est.y)/double(goal.x - Ball.x);
	double m2 = double(Robots::get_position(i).y - Ball.y)/double(Robots::get_position(i).x - Ball.x);
	double phi = atan((m2-m1)/(1+m2*m1));

	Robots::set_velocity(i);

	if(Robots::get_position(i).x > COORD_MID_FIELD_X
		&& abs(phi)*180/PI < 30) {
		Robots::set_status(i, ATK_PENALTI_STATE);
	}
	else if(Robots::get_position(atk).x > Ball.x
		&& Ball.x < COORD_MID_FIELD_X
		&& distance(Robots::get_position(atk), Ball) > 3*ABS_ROBOT_SIZE) {
		Robots::set_status(i, STEP_BACK);
	}
	else if(Ball.x < def_corner_line) {
		Robots::set_status(i, DEF_CORNER_STATE);
	}
	else {
		Robots::set_status(i, NORMAL_STATE);
	}

	switch(Robots::get_status(i)) {
		case NORMAL_STATE:

			Robots::set_cmdType(i, Robots::CMD::POSITION);

			// se a bola está no corner adversário, espera o cruzamento
			if(Ball_Est.x > corner_atk_limit) {
				int y_pos = Ball.y > COORD_GOAL_MID_Y ? COORD_GOAL_UP_Y : COORD_GOAL_DWN_Y;
				target = cv::Point(COORD_MID_FIELD_X + COORD_MID_FIELD_X/4, y_pos);

				Robots::set_target(i, target); // deve ser chamado antes do wait
				def_wait(i);
			}
			else if(Ball_Est.x > COORD_MID_FIELD_X) { // se a bola está depois do meio de campo, avança um pouco
                target = cv::Point(COORD_MID_FIELD_X - COORD_MID_FIELD_X/4, COORD_GOAL_MID_Y);

				Robots::set_target(i, target); // deve ser chamado antes do wait
				def_wait(i);
			}
			else { // senão, tenta tampar o angulo que o goleiro não consegue
				// fixa uma posição em x
				int x_pos = COORD_BOX_DEF_X + ABS_ROBOT_SIZE;

				int ballTarget = Ball.y > COORD_GOAL_MID_Y ? COORD_GOAL_UP_Y : COORD_GOAL_DWN_Y;
				double m = double(Ball.y - ballTarget) / double(Ball.x - ABS_ROBOT_SIZE);
				auto y_proj = int(Ball.y - m * (Ball.x - x_pos));

				// faz o crop do alvo se ele for muito discrepante (se a bola se aproximar muito do gol)
				if(y_proj > COORD_GOAL_DWN_Y) y_proj = COORD_GOAL_DWN_Y;
				else if(y_proj < COORD_GOAL_UP_Y) y_proj = COORD_GOAL_UP_Y;

				target = cv::Point(x_pos, y_proj);
			}

			break;

		case STEP_BACK:

			Robots::set_cmdType(i, Robots::CMD::POSITION);

			if(Ball.x > Robots::get_position(i).x + ABS_ROBOT_SIZE) {
				Robots::set_target(i, Ball);
			}
			else {
				pot_field_around(i);
				crop_targets(i);
			}

			break;

		case DEF_CORNER_STATE:

			Robots::set_cmdType(i, Robots::CMD::POSITION);

			if(Ball.y > COORD_GOAL_MID_Y) {
				target.x = goalie_line;
				target.y = COORD_BOX_DWN_Y + ABS_ROBOT_SIZE;
				if(distance(Ball, Robots::get_position(i)) < ABS_ROBOT_SIZE && Ball.y > Robots::get_position(i).y)
					Robots::spin_counter_clockwise(i, 0.8);
				// if(distance(Robots::get_position(i), Robots::get_target(i)) <= fixed_pos_distance) robots[i].target = Ball;
				if(is_near(i, target) && Ball.y > Robots::get_position(i).y) {
					target = Ball;
				}
			} else {
				target.x = goalie_line;
				target.y = COORD_BOX_UP_Y - ABS_ROBOT_SIZE;
				if(distance(Ball, Robots::get_position(i)) < ABS_ROBOT_SIZE && Ball.y < Robots::get_position(i).y)
					Robots::spin_clockwise(i, 0.8);
				// if(distance(Robots::get_position(i), Robots::get_target(i)) <= fixed_pos_distance) robots[i].target = Ball;
				if(is_near(i, target) && Ball.y < Robots::get_position(i).y) {
					target = Ball;
				}
			}

			Robots::set_target(i, target);

			break;

		case ATK_PENALTI_STATE:

			Robots::set_cmdType(i, Robots::CMD::VECTOR);
            Robots::kick(i, Ball);
    }
}

void Strategy::gk_routine(int i) {

    if((Ball.x < COORD_BOX_DEF_X && Ball.y < COORD_BOX_UP_Y) ||
            (Ball.x < COORD_BOX_DEF_X && Ball.y > COORD_BOX_DWN_Y)) { //Se está nos cantos, definir como CORNER_STATE;
        Robots::set_status(i, CORNER_STATE);
    }else if(){

    }

    cv::Point target;
	switch(Robots::get_status(i)) {
		case NORMAL_STATE:
			Robots::set_fixedPos(i, true);

            Robots::set_cmdType(i, Robots::CMD::POSITION);

			target.x = goalie_line;
			target.y = Ball.y;

			// if bola com velocidade && vindo pra defesa && fora da area
			if(distance(Ball, Ball_Est) > ABS_ROBOT_SIZE && Ball.x > Ball_Est.x && (Ball.x > COORD_BOX_DEF_X)) {
				double m = double(Ball.y - Ball_Est.y)/double(Ball.x - Ball_Est.x);
				auto pred_y = int(Ball.y - m * (Ball.x - (goalie_line + ABS_ROBOT_SIZE)));

				if(pred_y > COORD_GOAL_DWN_Y)
					pred_y = COORD_GOAL_DWN_Y;

				if(pred_y < COORD_GOAL_UP_Y)
					pred_y = COORD_GOAL_UP_Y;

				if(Ball.x > COORD_MID_FIELD_X) {
					target.y = pred_y;
				} else {
					cv::Point v = cv::Point(0, Ball.y - pred_y); // vetor da predição à projeção (na linha do gol)
					double vector_module = sqrt(pow(v.x,2) + pow(v.y,2)); // modulo

					double approach = abs(Ball.y - pred_y) * (Ball.x - (Robots::get_position(i).x + ABS_ROBOT_SIZE))/float(COORD_MID_FIELD_X - Robots::get_position(i).x);

					target.y = int(pred_y + (v.y/vector_module) * approach);
				}
			} else {
				target.y = Ball.y;
			}

			if(target.y > COORD_GOAL_DWN_Y) target.y = COORD_GOAL_DWN_Y - ABS_ROBOT_SIZE;

			if(target.y < COORD_GOAL_UP_Y) target.y = COORD_GOAL_UP_Y + ABS_ROBOT_SIZE;

			// if(Ball.x < COORD_BOX_DEF_X && Ball.y > COORD_GOAL_DWN_Y) {
			// 	// if bola no corner inferior do gol
			// 	robots[i].target.y = COORD_GOAL_DWN_Y + goalie_offset;
			// } else if (Ball.x < COORD_BOX_DEF_X && Ball.y < COORD_GOAL_UP_Y){
			// 	// if bola no corner superior do gol
			// 	robots[i].target.y = COORD_GOAL_UP_Y - goalie_offset;
			// }

			// if (Ball.x < COORD_BOX_DEF_X && Ball.y > COORD_BOX_UP_Y && Ball.y < COORD_GOAL_DWN_Y) {
			// 	Robots::set_target(i, Ball);
			// }

        	//Robots::set_velocity(i, float(0.3 + ((Robots::get_default_role_velocity(GOALKEEPER)-0.3) * (distance(Robots::get_position(i), Robots::get_target(i)))/ABS_GOAL_SIZE_Y)));
			// cout << "vmax " << robots[i].vmax << " distancia "<< distance(Robots::get_position(i), Robots::get_target(i)) << " distancia max " << ABS_GOAL_TO_GOAL_WIDTH/4<<endl;

			/*if(Robots::get_velocity(i) > Robots::get_default_velocity(i))
                Robots::set_velocity(i);

			if(Robots::get_velocity(i) < 0.35)
                Robots::set_velocity(i, 0.35f);*/

			// fixed_lookup(i);

			if(distance(Robots::get_position(i), Ball) < ABS_ROBOT_SIZE*1.3) {
				if(Ball.y <= COORD_GOAL_UP_Y) {
					Robots::spin_clockwise(i, 1.f);
				} else if(Ball.y >= COORD_GOAL_DWN_Y) {
					Robots::spin_counter_clockwise(i, 1.f);
				}
			}

			if(distance(Robots::get_position(i), Ball) < ABS_ROBOT_SIZE*1.3) {
				if(Ball.y <= Robots::get_position(i).y) {
					Robots::spin_clockwise(i, 1.f);
				} else if(Ball.y >= Robots::get_position(i).y){
					Robots::spin_counter_clockwise(i, 1.f);
				}
			}
			// chute pra tirar a bola da defesa
			if(abs(Ball.x - Robots::get_position(i).x) <= ABS_ROBOT_SIZE && Ball.y < COORD_BOX_DWN_Y && Ball.y > COORD_BOX_UP_Y) {
				if (Ball.y <= Robots::get_position(i).y) {
					target.y = 0;
					target.x = goalie_line; //  + ABS_ROBOT_SIZE/2
					Robots::set_velocity(i, MAX_VEL);
				} else {
					target.y = 480;
					target.x = goalie_line; //  + ABS_ROBOT_SIZE/2
					Robots::set_velocity(i, MAX_VEL);
				}
			}

			Robots::set_target(i, target);

			if(distance(Robots::get_position(i), Robots::get_target(i)) < ABS_ROBOT_SIZE*3.0/4) {
                Robots::look_at(i, PI/2);
			}

			break;
		case CORNER_STATE:
            Robots::set_velocity(i, MAX_VEL);
            target.x = goalie_line;
            if(Ball.y > COORD_BOX_DWN_Y){
                target.y = COORD_GOAL_DWN_Y;
            }else{
                target.y = COORD_GOAL_UP_Y;
            }

            //Chutar para fora
            //Ativar transição?


            break;
        case BALL_IN_AREA_STATE:
        	//Posicionar o robô atrás da bola;
            //!TODO Ver se o UVF resolve o posicionamento.

        case INVIABLE_TRANSITION_STATE:
        	//Gira com a velocidade máxima

            target.x = goalie_line;
            target.y = Ball.y;

            if(target.y > COORD_GOAL_DWN_Y) target.y = COORD_GOAL_DWN_Y - ABS_ROBOT_SIZE;
            if(target.y < COORD_GOAL_UP_Y) target.y = COORD_GOAL_UP_Y + ABS_ROBOT_SIZE;


            if(distance(Robots::get_position(i), Ball) < ABS_ROBOT_SIZE*1.3) {
                if(Ball.y <= COORD_GOAL_UP_Y) {
                    Robots::spin_clockwise(i, 1.f);
                } else if(Ball.y >= COORD_GOAL_DWN_Y) {
                    Robots::spin_counter_clockwise(i, 1.f);
                }
            }

            if(distance(Robots::get_position(i), Ball) < ABS_ROBOT_SIZE*1.3) {
                if(Ball.y <= Robots::get_position(i).y) {
                    Robots::spin_clockwise(i, 1.f);
                } else if(Ball.y >= Robots::get_position(i).y){
                    Robots::spin_counter_clockwise(i, 1.f);
                }
            }
    }
}

void Strategy::set_Ball(cv::Point b) {
    Ball = b;
    LS_ball_x.addValue(Ball.x);
    LS_ball_y.addValue(Ball.y);

	Ball_Est.x = static_cast<int>(LS_ball_x.estimate(10));
	Ball_Est.y = static_cast<int>(LS_ball_y.estimate(10));
}

cv::Point Strategy::get_Ball_Est() {
    return Ball_Est;
}

double Strategy::distance(cv::Point A, cv::Point B) {
	double dist = sqrt(pow(double(A.y - B.y), 2) + pow(double(A.x - B.x), 2));
	return dist;
}

double Strategy::distance_meters(cv::Point A, cv::Point B) {
	double dist = sqrt(pow(double(A.y - B.y), 2) + pow(double(A.x - B.x), 2));
	dist = 1.7*dist/ABS_GOAL_TO_GOAL_WIDTH;
	return dist;
}

void Strategy::get_variables() {
	goalie_line = (int)testFrame.getValue(0); // index da barrinha
	goalie_offset = (int)testFrame.getValue(1); // index da barrinha
}

void Strategy::get_past(int i) {
	past_position[i] = Robots::get_position(i);
	past_transangle[i] = Robots::get_transAngle(i);
}
