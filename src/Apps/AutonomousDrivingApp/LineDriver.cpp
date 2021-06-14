#include "LineDriver.h"
#include "../../Components/ImageProc.h"

LineDriver::LineDriver() : Driver(){
}

void LineDriver::process(){
	thinningBuffer =  (Color*)ps_malloc(160 * 120 * sizeof(Color));
	memcpy(thinningBuffer, getCameraImage(), 160 * 120 * sizeof(Color));

	//B/W contrasting
	ImageProc::contrast(thinningBuffer, 160, 120, thinningBuffer, 122);

	skeleton_tracer_t* T = new skeleton_tracer_t();
	T->W = 160;
	T->H = 120;
	T->im = thinningBuffer;

	//algorithm expects binary image
	for(uint i = 0; i < T->H*T->H; i++){
		thinningBuffer[i] &= 1;
	}
	T->thinning_zs();

	skeleton_tracer_t::polyline_t* p = (skeleton_tracer_t::polyline_t*)T->trace_skeleton(0, 0, T->W, T->H, 0);

	//iterate through polylines and find the longest
	skeleton_tracer_t::polyline_t* pl = p;
	float maxLength = 0;
	skeleton_tracer_t::polyline_t* longestPolyPtr;
	while(pl){
		float polylineLength = 0;
		skeleton_tracer_t::point_t* point = pl->head;
		while(point->next){
			polylineLength += sqrt(pow(point->x - point->next->x, 2) + pow(point->y - point->next->y, 2));
			point = point->next;
		}
		if(maxLength < polylineLength){
			maxLength = polylineLength;
			longestPolyPtr = pl;
		}
		pl = pl->next;
	}

	//copy longest polyline, point by point
	skeleton_tracer_t::point_t* copyPoint = new skeleton_tracer_t::point_t(*longestPolyPtr->head);
	line.head = copyPoint;
	for(skeleton_tracer_t::point_t* point = longestPolyPtr->head->next; point != nullptr; point = point->next){
		copyPoint->next = new skeleton_tracer_t::point_t(*point);
		copyPoint = copyPoint->next;
		drawLine(point->x, point->y, point->next->x, point->next->y, getCameraImage(),TFT_RED);
	}

	copyPoint->next = nullptr;
	line.tail = copyPoint;

	// clean up
	T->destroy_polylines(p);
	T->destroy_rects();
	delete T;
	free(thinningBuffer);
}

Color* LineDriver::getCameraImage(){
	return Driver::getCameraImage();
}

void LineDriver::drawLine(int x1, int y1, int x2, int y2, Color* buffer,uint32_t color){

        // Bresenham's line algorithm
  const bool steep = (fabs(y2 - y1) > fabs(x2 - x1));
  if(steep)
  {
    std::swap(x1, y1);
    std::swap(x2, y2);
  }

  if(x1 > x2)
  {
    std::swap(x1, x2);
    std::swap(y1, y2);
  }

  const float dx = x2 - x1;
  const float dy = fabs(y2 - y1);

  float error = dx / 2.0f;
  const int ystep = (y1 < y2) ? 1 : -1;
  int y = (int)y1;

  const int maxX = (int)x2;

  for(int x=(int)x1; x<=maxX; x++)
  {
    if(steep)
    {
        buffer[x*120+y]=color;
    }
    else
    {
       buffer[y*160+x]=color;
    }

    error -= dy;
    if(error < 0)
    {
        y += ystep;
        error += dx;
    }
  }
}
