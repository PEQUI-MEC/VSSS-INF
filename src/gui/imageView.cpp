#include "imageView.hpp"

bool ImageView::on_button_press_event(GdkEventButton *event) {
	if(split_flag) {
		if(event->x <= width/2 && event->y <= height/2) {
			sector = 0;
		} else if(event->x <= width/2 && event->y > height/2) {
			sector = 2;
		} else if(event->x > width/2 && event->y <= height/2) {
			sector = 1;
		} else if(event->x > width/2 && event->y > height/2) {
			sector = 3;
		}
	}

	if(warp_event_flag) {
		//std::cerr <<"EVENT"<<std::endl;
		if(event->button == 1) {
			//std::cerr << event->x <<"	"<< event->y<< std::endl;
			warp_mat[warp_counter][0] =  event->x;
			warp_mat[warp_counter][1] =  event->y;
			warp_counter++;

			if(warp_counter == 4){
				//std::cerr <<"COUNTER END"<<std::endl;
				warp_event_flag = false;
				hold_warp = true;
				warp_counter=0;
			}
			return true;
		}
		return true;
	}

	if(adjust_event_flag) {
		if(event->button == 1) {
			adj_counter++;
			//std::cerr << event->x <<"	"<< event->y<< std::endl;
			std::cerr << adj_counter << std::endl;

			if(event->x < width/2) {
				if(event->y < height/2) {
					//std::cerr <<"ESQUERDA SUPERIOR"<<std::endl;
					adjust_mat[0][0] = event->x;
					adjust_mat[0][1] = event->y;
				} else {
					//std::cerr <<"ESQUERDA INFERIOR"<<std::endl;
					adjust_mat[1][0] = event->x;
					adjust_mat[1][1] = event->y;
				}
			} else {
				if(event->y < height/2) {
					//std::cerr <<"DIREITA SUPERIOR"<<std::endl;
					adjust_mat[2][0] = event->x;
					adjust_mat[2][1] = event->y;
				} else {
					//std::cerr <<"DIREITA INFERIOR"<<std::endl;
					adjust_mat[3][0] = event->x;
					adjust_mat[3][1] = event->y;
				}
			}
			if(adj_counter == 4){
				adjust_rdy = true;
				adjust_event_flag = false;
				adj_counter = 0;
				//std::cerr << "ADJ END"<< std::endl;
			}
		}
	} else if(PID_test_flag) {
		robot_pos[0] = 0;
		robot_pos[1] = 0;
		if(event->button == 1) {
			robot_pos[0] = event->x;
			robot_pos[1] = event->y;
			//std::cerr <<robot_pos[0] <<"  -  "<<robot_pos[1]<<std::endl;
		} else if(event->button == 3) {
			tar_pos[0] = event->x;
			tar_pos[1] = event->y;
			//std::cerr <<tar_pos[0] <<"  -  "<<tar_pos[1]<<std::endl;
		}
	} else if(formation_flag) {
		robot_pos[0]= -1;
		robot_pos[1]= -1;

		if(event->button == 1) {
			robot_pos[0] = event->x;
			robot_pos[1] = event->y;

			//std::cerr <<robot_pos[0] <<"  -  "<<robot_pos[1]<<std::endl;
		} else if(event->button == 2) {
			look_pos[0] = event->x;
			look_pos[1] = event->y;
		} else if(event->button == 3) {
			tar_pos[0] = event->x;
			tar_pos[1] = event->y;
			//std::cerr <<tar_pos[0] <<"  -  "<<tar_pos[1]<<std::endl;
		}
	}
}

ImageView::ImageView() : data(0), width(0), height(0), stride(0) {
	robot_pos[1] = 0;
	robot_pos[0] = 0;
	tar_pos[1] = -1;
	tar_pos[0] = -1;
}

void ImageView::set_data(unsigned char * data, int width, int height) {
	this->data = data;
	this->width = width;
	this->height = height;
	this->stride = width * 3;
}

unsigned char * ImageView::get_data() {
	return this->data;
}

int ImageView::get_width() {
	return this->width;
}

int ImageView::get_height() {
	return this->height;
}

void ImageView::disable_image_show() {
	data = 0;
	width = 0;
	height = 0;
	stride = 0;
}

void ImageView::refresh() {
	this->queue_draw();
}

bool ImageView::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
	add_events(Gdk::BUTTON_PRESS_MASK);
	if(!data) return false;

	pb = Gdk::Pixbuf::create_from_data(data, Gdk::COLORSPACE_RGB, false, 8, width, height, stride);
	Gdk::Cairo::set_source_pixbuf(cr, pb, 0, 0);
	cr->paint();

//				cr->set_line_width(5.0);
//				cr->set_source_rgb(1, 0, 0);
//				cr->move_to(0, 0);
//				cr->line_to(width, height);
//				cr->stroke();
//
//				// http://developer.gnome.org/pangomm/unstable/classPango_1_1FontDescription.html
//				Pango::FontDescription font;
//				font.set_family("Monospace");
//				font.set_weight(Pango::WEIGHT_BOLD);
//				// http://developer.gnome.org/pangomm/unstable/classPango_1_1Layout.html
//				Glib::RefPtr<Pango::Layout> layout = create_pango_layout("Hi there!");
//				layout->set_font_description(font);
//				int text_width;
//				int text_height;
//				//get the text dimensions (it updates the variables -- by reference)
//				layout->get_pixel_size(text_width, text_height);
//				// Position the text in the middle
//				cr->move_to(10, 10);
//				layout->show_in_cairo_context(cr);

	return true;
}
