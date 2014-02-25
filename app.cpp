#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Polygon_2.h>
#include <vector>
#include <iostream>

/*oi domes stis opoies apothikevode oi plirofories gia ta faces kai ta vertices*/

struct FaceInfo2
{
    FaceInfo2(){}
    int nesting_level;
    bool done;

    bool in_domain(){
        return nesting_level%2 == 1;
    }
    bool is_done(){
        return done;
    }
};

struct VertexInfo{

    VertexInfo(){}
    int color; // 1 = Kokkino, 2 = Mple, 3 = Prasino
    bool colored; /* simbolizei to an exei xrwmatistei i korifi */
    bool can_change;
    int the_id;  /* simbolizei to id tou kathe vertex,to opoio einai monadiko */

    int get_color(){
        return color;
    }
    bool get_colored(){
        return colored;
    }
    bool get_can_change(){
        return can_change;
    }
    int get_the_id(){
        return the_id;
    }
};

typedef CGAL::Exact_predicates_inexact_constructions_kernel       K;
typedef CGAL::Triangulation_vertex_base_with_info_2<VertexInfo,K> Vbb;
typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2,K>    Fbb;
typedef CGAL::Constrained_triangulation_face_base_2<K,Fbb>        Fb;
typedef CGAL::Triangulation_data_structure_2<Vbb,Fb>               TDS;
typedef CGAL::Exact_predicates_tag                                Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag>  CDT;
typedef CDT::Point                                                Point;
typedef CGAL::Polygon_2<K>                                        Polygon_2;
typedef CDT::Face_iterator Face_iterator;

/* i trigonopiisi enos aplou poligonou odigei sekirto polugwno. Synepws xrisimopoioume aftes tis
   duo sinartiseis gia na aferesoume ta trigwna ta opoia dn apoteloun meros tu aplou polugwnou.*/

void mark_domains(CDT& ct,
             CDT::Face_handle start,
             int index,
             std::list<CDT::Edge>& border )
{
    if(start->info().nesting_level != -1){
        return;
    }
    std::list<CDT::Face_handle> queue;
    queue.push_back(start);

    while(! queue.empty()){
        CDT::Face_handle fh = queue.front();
        queue.pop_front();
        if(fh->info().nesting_level == -1){
            fh->info().nesting_level = index;
            for(int i = 0; i < 3; i++){
                CDT::Edge e(fh,i);
                CDT::Face_handle n = fh->neighbor(i);
                if(n->info().nesting_level == -1){
                    if(ct.is_constrained(e)) border.push_back(e);
                    else queue.push_back(n);
                }
            }
        }
    }
}


void mark_domains(CDT& cdt)
{
    for(CDT::All_faces_iterator it = cdt.all_faces_begin(); it != cdt.all_faces_end(); ++it){
        it->info().nesting_level = -1;
    }

    int index = 0;
    std::list<CDT::Edge> border;
    mark_domains(cdt, cdt.infinite_face(), index++, border);
    while(! border.empty()){
        CDT::Edge e = border.front();
        border.pop_front();
        CDT::Face_handle n = e.first->neighbor(e.second);
        if(n->info().nesting_level == -1){
            mark_domains(cdt, n, e.first->info().nesting_level+1, border);
        }
    }
}

/* edw ginetai i constrained triangulation */
void insert_polygon(CDT& cdt,const Polygon_2& polygon){
    if ( polygon.is_empty() ) return;
    CDT::Vertex_handle v_prev=cdt.insert(*CGAL::cpp0x::prev(polygon.vertices_end()));
    for (Polygon_2::Vertex_iterator vit=polygon.vertices_begin();
         vit!=polygon.vertices_end();++vit)
    {
        CDT::Vertex_handle vh=cdt.insert(*vit);
        cdt.insert_constraint(vh,v_prev);
        v_prev=vh;
    }
}

/*epistrefei ean ena stixeio iparxei se ena vector. Xrisimopoithike vector gia na apothikevetai to vertex id sto xrwma pu anikei*/
bool is_inside(int x, std::vector<int> kati){
    int count = 0;
    for (int i = 0; i <= kati.size(); i++) {
        if(x == kati[i]){
            count++;
            break;
        }
    }

    if(count > 0){
        return true;
    }else{
    return false;
    }
}

int main( )
{
    /*eisagwgi simeiwn*/
    Polygon_2 polygon1;
    polygon1.push_back(Point(2,5));
    polygon1.push_back(Point(3,3));
    polygon1.push_back(Point(2,1));
    polygon1.push_back(Point(5,1));
    polygon1.push_back(Point(6,3));
    polygon1.push_back(Point(6,2));
    polygon1.push_back(Point(8,1));
    polygon1.push_back(Point(9,4));
    polygon1.push_back(Point(10,4));
    polygon1.push_back(Point(10,1));
    polygon1.push_back(Point(12,3));
    polygon1.push_back(Point(10,8));
    polygon1.push_back(Point(7,8));
    polygon1.push_back(Point(5,9));
    polygon1.push_back(Point(4,8));
    polygon1.push_back(Point(6,8));
    polygon1.push_back(Point(5,7));
    polygon1.push_back(Point(3,6));
    polygon1.push_back(Point(1,10));



    CDT cdt;
    insert_polygon(cdt,polygon1);
    mark_domains(cdt);


    int the_id = 1;
     int facelets_count = 0;
    /*arxikopoiiseis colored,can-change(den xrisimopoiithike) kai twn vertex id
     Episis vriskoume posa faces exoun dimiurgithei*/
    for (CDT::Finite_faces_iterator fit=cdt.finite_faces_begin();
         fit!=cdt.finite_faces_end();++fit)
    {
        if ( fit->info().in_domain() ){
            std::cout << the_id <<  std::endl;
            fit->info().done = false;
            fit->vertex(0)->info().colored = false;
            fit->vertex(1)->info().colored = false;
            fit->vertex(2)->info().colored = false;

            fit->vertex(0)->info().can_change = true;
            fit->vertex(1)->info().can_change = true;
            fit->vertex(2)->info().can_change = true;


            fit->vertex(0)->info().the_id = the_id;
            the_id++;
            fit->vertex(1)->info().the_id = the_id;
            the_id++;
            fit->vertex(2)->info().the_id = the_id;
            the_id++;

             facelets_count++;
        }

    }


    int count=0;

   /*dimiurgume ta vector*/
    std::vector<int> red;
    std::vector<int> blue;
    std::vector<int> yellow;
    for (int k=0; k<=facelets_count; k++) {


    for (CDT::Finite_faces_iterator fit=cdt.finite_faces_begin();
         fit!=cdt.finite_faces_end();++fit)
    {
        if ( fit->info().in_domain() ){
            //std::cout << count << std::endl;


            if(count == 0){ /*An den exei mpei pote apla xromatiseta ola ta shmeia*/

                fit->vertex(0)->info().color = 1;
                red.push_back(fit->vertex(0)->info().the_id);
                fit->vertex(1)->info().color = 2;
                blue.push_back(fit->vertex(1)->info().the_id);
                fit->vertex(2)->info().color = 3;
                yellow.push_back(fit->vertex(2)->info().the_id);

                fit->vertex(0)->info().colored = true;
                fit->vertex(1)->info().colored = true;
                fit->vertex(2)->info().colored = true;
                fit->info().done = true;

            }else{
                /*An to trigwno den exei ginei done, diladi den exei xwmatistei plirws, prospathei na brei tus sindiadsmous twn xrwmatwn
                  gia na vapsei tin epomeni koryfi. Ta if xrisimopoioude gia na vrethun oloi oi sindiasmoi*/
                if(fit->info().done == false  ){

                    if (is_inside(fit->vertex(0)->info().the_id, blue) == true) {// BLUE

                    if (is_inside(fit->vertex(1)->info().the_id, red) == true) {

                        if (is_inside(fit->vertex(2)->info().the_id, yellow) == false) {
                             fit->vertex(2)->info().color = 3;
                             yellow.push_back(fit->vertex(2)->info().the_id);
                             fit->info().done = true;
                            continue;

                        }else{
                             fit->info().done = true;
                            continue;
                        }


                    }else if(is_inside(fit->vertex(1)->info().the_id, yellow) == true){

                        if (is_inside(fit->vertex(2)->info().the_id, red) == false) {
                            fit->vertex(2)->info().color = 1;
                            red.push_back(fit->vertex(2)->info().the_id);
                            fit->info().done = true;
                            continue;

                        }else{
                            fit->info().done = true;
                            continue;
                        }
                    }else if(is_inside(fit->vertex(2)->info().the_id, red) == true){

                        if (is_inside(fit->vertex(1)->info().the_id, yellow) == false) {
                            fit->vertex(1)->info().color = 3;
                        yellow.push_back(fit->vertex(1)->info().the_id);
                            fit->info().done = true;
                            continue;
                        }else{
                            fit->info().done = true;
                            continue;
                        }
                    }else if(is_inside(fit->vertex(2)->info().the_id, yellow) == true){

                        if (is_inside(fit->vertex(1)->info().the_id, red) == false) {
                            fit->vertex(1)->info().color = 1;
                            red.push_back(fit->vertex(1)->info().the_id);
                            fit->info().done = true;
                            continue;
                        }else{
                            fit->info().done = true;
                            continue;
                        }
                    }
                    }
                        if (is_inside(fit->vertex(0)->info().the_id, red) == true) {  // RED
                        if (is_inside(fit->vertex(1)->info().the_id, blue) == true) {
                            if (is_inside(fit->vertex(2)->info().the_id, yellow) == false) {
                                fit->vertex(2)->info().color = 3;
                                yellow.push_back(fit->vertex(2)->info().the_id);
                                fit->info().done = true;
                                continue;
                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        }else if(is_inside(fit->vertex(1)->info().the_id, yellow) == true){
                            if (is_inside(fit->vertex(2)->info().the_id, blue) == false) {
                                fit->vertex(2)->info().color = 2;
                                blue.push_back(fit->vertex(2)->info().the_id);
                                fit->info().done = true;
                                continue;
                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        }else if(is_inside(fit->vertex(2)->info().the_id, blue) == true){
                            if (is_inside(fit->vertex(1)->info().the_id, yellow) == false) {
                                fit->vertex(1)->info().color = 3;
                                yellow.push_back(fit->vertex(1)->info().the_id);
                                fit->info().done = true;
                                continue;
                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        }else if(is_inside(fit->vertex(2)->info().the_id, yellow) == true){
                            if (is_inside(fit->vertex(1)->info().the_id, blue) == false) {
                                fit->vertex(1)->info().color = 2;
                               blue.push_back(fit->vertex(1)->info().the_id);
                                fit->info().done = true;
                                continue;

                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        }
                    }//
                    if (is_inside(fit->vertex(0)->info().the_id, yellow) == true) { // YELLOW
                        if (is_inside(fit->vertex(1)->info().the_id, red) == true) {
                            if (is_inside(fit->vertex(2)->info().the_id, blue) == false) {
                                fit->vertex(2)->info().color = 2;
                                blue.push_back(fit->vertex(2)->info().the_id);
                                fit->info().done = true;
                                continue;
                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        }else if(is_inside(fit->vertex(1)->info().the_id, blue) == true){

                            if (is_inside(fit->vertex(2)->info().the_id, red) == false) {
                                fit->vertex(2)->info().color = 1;
                                red.push_back(fit->vertex(2)->info().the_id);
                                fit->info().done = true;
                                continue;
                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        }else if(is_inside(fit->vertex(2)->info().the_id, red) == true){
                            if (is_inside(fit->vertex(1)->info().the_id, blue) == false) {
                                fit->vertex(1)->info().color = 2;
                                blue.push_back(fit->vertex(1)->info().the_id);
                                fit->info().done = true;
                                continue;

                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        }else if(is_inside(fit->vertex(2)->info().the_id, blue) == true){
                            if (is_inside(fit->vertex(1)->info().the_id, red) == false) {
                                fit->vertex(1)->info().color = 1;
                                red.push_back(fit->vertex(1)->info().the_id);
                                fit->info().done = true;
                                continue;
                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        }
                    } else if(is_inside(fit->vertex(1)->info().the_id, blue) == true){ //BLUE
                          if (is_inside(fit->vertex(0)->info().the_id, red) == true) {
                              if (is_inside(fit->vertex(2)->info().the_id, yellow) == false) {
                                  fit->vertex(2)->info().color = 3;
                                  yellow.push_back(fit->vertex(2)->info().the_id);
                                  fit->info().done = true;
                                  continue;
                              }else{
                                  fit->info().done = true;
                                  continue;
                              }
                          } else if (is_inside(fit->vertex(0)->info().the_id,yellow) == true) {

                              if (is_inside(fit->vertex(2)->info().the_id, red) == false) {
                                  fit->vertex(2)->info().color = 1;
                                  red.push_back(fit->vertex(2)->info().the_id);
                                  fit->info().done = true;
                                  continue;

                              }else{
                                  fit->info().done = true;
                                  continue;
                              }
                          }  else if (is_inside(fit->vertex(2)->info().the_id, red) == true) {

                              if (is_inside(fit->vertex(0)->info().the_id, yellow) == false) {
                                  fit->vertex(0)->info().color = 3;
                                  yellow.push_back(fit->vertex(0)->info().the_id);
                                  fit->info().done = true;
                                  continue;

                              }else{
                                  fit->info().done = true;
                                  continue;
                              }
                          } else if (is_inside(fit->vertex(2)->info().the_id, yellow) == true) {

                              if (is_inside(fit->vertex(0)->info().the_id, red) == false) {
                                  fit->vertex(0)->info().color = 1;
                                  red.push_back(fit->vertex(0)->info().the_id);
                                  fit->info().done = true;
                                  continue;

                              }else{
                                  fit->info().done = true;
                                  continue;
                              }
                          }
                    }  else if(is_inside(fit->vertex(1)->info().the_id, red) == true){ //RED

                        if (is_inside(fit->vertex(0)->info().the_id, blue) == true) {

                            if (is_inside(fit->vertex(2)->info().the_id, yellow) == false) {
                                fit->vertex(2)->info().color = 3;
                                yellow.push_back(fit->vertex(2)->info().the_id);
                                fit->info().done = true;
                                continue;

                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        } else if (is_inside(fit->vertex(0)->info().the_id, yellow) == true) {

                            if (is_inside(fit->vertex(2)->info().the_id, blue) == false) {
                                fit->vertex(2)->info().color = 2;
                                blue.push_back(fit->vertex(2)->info().the_id);
                                fit->info().done = true;
                                continue;
                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        }  else if (is_inside(fit->vertex(2)->info().the_id, blue) == true) {

                            if (is_inside(fit->vertex(0)->info().the_id, yellow) == false) {
                                fit->vertex(0)->info().color = 3;
                                yellow.push_back(fit->vertex(0)->info().the_id);
                                fit->info().done = true;
                                continue;

                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        } else if (is_inside(fit->vertex(2)->info().the_id, yellow) == true) {

                            if (is_inside(fit->vertex(0)->info().the_id, blue) == false) {
                                fit->vertex(0)->info().color = 2;
                                blue.push_back(fit->vertex(0)->info().the_id);
                                fit->info().done = true;
                                continue;

                            }else{
                                fit->info().done = true;
                                continue;
                            }

                        }
                    }
                   if(is_inside(fit->vertex(1)->info().the_id, yellow) == true){ //Yellow

                       if (is_inside(fit->vertex(0)->info().the_id, red) == true) {

                           if (is_inside(fit->vertex(2)->info().the_id, blue) == false) {
                               fit->vertex(2)->info().color = 2;
                               blue.push_back(fit->vertex(2)->info().the_id);
                               fit->info().done = true;
                               continue;

                           }else{
                               fit->info().done = true;
                               continue;
                           }



                       } else if (is_inside(fit->vertex(0)->info().the_id, blue) == true) {

                           if (is_inside(fit->vertex(2)->info().the_id, red) == false) {
                               fit->vertex(2)->info().color = 1;
                               red.push_back(fit->vertex(2)->info().the_id);
                               fit->info().done = true;
                               continue;

                           }else{
                               fit->info().done = true;
                               continue;
                           }

                       }  else if (is_inside(fit->vertex(2)->info().the_id, red) == true) {

                           if (is_inside(fit->vertex(0)->info().the_id, blue) == false) {
                               fit->vertex(0)->info().color = 2;
                               blue.push_back(fit->vertex(0)->info().the_id);
                               fit->info().done = true;
                               continue;

                           }else{
                               fit->info().done = true;
                               continue;
                           }

                       } else if (is_inside(fit->vertex(2)->info().the_id, blue) == true) {

                           if (is_inside(fit->vertex(0)->info().the_id, red) == false) {
                               fit->vertex(0)->info().color = 1;
                               red.push_back(fit->vertex(0)->info().the_id);
                               fit->info().done = true;
                               continue;

                           }else{
                               fit->info().done = true;
                               continue;
                           }

                       }
                   } //////////////////////////////////////////////////////////
                    if(is_inside(fit->vertex(2)->info().the_id, blue) == true){ // BLUE
                        if (is_inside(fit->vertex(1)->info().the_id, red) == true) {

                            if (is_inside(fit->vertex(0)->info().the_id, yellow) == false) {
                                fit->vertex(0)->info().color = 3;
                                yellow.push_back(fit->vertex(0)->info().the_id);
                                fit->info().done = true;
                                continue;

                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        } else if (is_inside(fit->vertex(1)->info().the_id, yellow) == true) {

                            if (is_inside(fit->vertex(0)->info().the_id, red) == false) {
                                fit->vertex(0)->info().color = 1;
                                red.push_back(fit->vertex(0)->info().the_id);
                                fit->info().done = true;
                                continue;

                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        }  else if (is_inside(fit->vertex(0)->info().the_id, red) == true) {

                            if (is_inside(fit->vertex(1)->info().the_id,yellow) == false) {
                                fit->vertex(1)->info().color = 3;
                                yellow.push_back(fit->vertex(1)->info().the_id);
                                fit->info().done = true;
                                continue;

                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        } else if (is_inside(fit->vertex(0)->info().the_id, yellow) == true) {

                            if (is_inside(fit->vertex(1)->info().the_id, red) == false) {
                                fit->vertex(1)->info().color = 1;
                                red.push_back(fit->vertex(1)->info().the_id);
                                fit->info().done = true;
                                continue;

                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        }
                            if(is_inside(fit->vertex(2)->info().the_id, red) == true){ //RED

                            if (is_inside(fit->vertex(1)->info().the_id, blue) == true) {

                                if (is_inside(fit->vertex(0)->info().the_id, yellow) == false) {
                                    fit->vertex(0)->info().color = 3;
                                    yellow.push_back(fit->vertex(0)->info().the_id);
                                    fit->info().done = true;
                                    continue;

                                }else{
                                    fit->info().done = true;
                                    continue;
                                }
                            } else if (is_inside(fit->vertex(1)->info().the_id, yellow) == true) {

                                if (is_inside(fit->vertex(0)->info().the_id, blue) == false) {
                                    fit->vertex(0)->info().color = 2;
                                    blue.push_back(fit->vertex(0)->info().the_id);
                                    fit->info().done = true;
                                    continue;

                                }else{
                                    fit->info().done = true;
                                    continue;
                                }
                            }  else if (is_inside(fit->vertex(0)->info().the_id, blue) == true) {

                                if (is_inside(fit->vertex(1)->info().the_id,yellow) == false) {
                                    fit->vertex(1)->info().color = 3;
                                    yellow.push_back(fit->vertex(1)->info().the_id);
                                    fit->info().done = true;
                                    continue;

                                }else{
                                    fit->info().done = true;
                                    continue;
                                }
                            } else if (is_inside(fit->vertex(0)->info().the_id, yellow) == true) {

                                if (is_inside(fit->vertex(1)->info().the_id, blue) == false) {
                                    fit->vertex(1)->info().color = 2;
                                    blue.push_back(fit->vertex(1)->info().the_id);
                                    fit->info().done = true;
                                    continue;

                                }else{
                                    fit->info().done = true;
                                    continue;
                                }
                            }
                        }
                    }
                    if(is_inside(fit->vertex(2)->info().the_id, yellow) == true){ // YELLOW
                        if (is_inside(fit->vertex(1)->info().the_id, red) == true) {

                            if (is_inside(fit->vertex(0)->info().the_id, blue) == false) {
                                fit->vertex(0)->info().color = 2;
                                blue.push_back(fit->vertex(0)->info().the_id);
                                fit->info().done = true;
                                continue;

                            }else{
                                fit->info().done = true;
                                continue;
                            }

                        } else if (is_inside(fit->vertex(1)->info().the_id, blue) == true) {

                            if (is_inside(fit->vertex(0)->info().the_id, red) == false) {
                                fit->vertex(0)->info().color = 1;
                                red.push_back(fit->vertex(0)->info().the_id);
                                fit->info().done = true;
                                continue;

                            }else{
                                fit->info().done = true;
                                continue;
                            }

                        }  else if (is_inside(fit->vertex(0)->info().the_id, red) == true) {

                            if (is_inside(fit->vertex(1)->info().the_id,blue) == false) {
                                fit->vertex(1)->info().color = 2;
                                blue.push_back(fit->vertex(1)->info().the_id);
                                fit->info().done = true;
                                continue;

                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        } else if (is_inside(fit->vertex(0)->info().the_id, blue) == true) {

                            if (is_inside(fit->vertex(1)->info().the_id, red) == false) {
                                fit->vertex(1)->info().color = 1;
                                red.push_back(fit->vertex(1)->info().the_id);
                                fit->info().done = true;
                                continue;

                            }else{
                                fit->info().done = true;
                                continue;
                            }
                        }
                    }
                }
            }
         count++;
        }
    }
    }
    /*Ektypwseis*/
    int zozela =0;

    for (CDT::Finite_faces_iterator fit=cdt.finite_faces_begin();
         fit!=cdt.finite_faces_end();++fit)
    {
        if ( fit->info().in_domain() ){
            std::cout << zozela << std::endl;
            //     std::cout << fit->vertex(0)->info().get_color() << std::endl;
            std::cout << fit->vertex(0)->point() << " color: " << fit->vertex(0)->info().get_color() << " - " << fit->vertex(0)->info().get_the_id() << std::endl;
            std::cout << fit->vertex(1)->point() << " color: " << fit->vertex(1)->info().get_color()  << " - " << fit->vertex(1)->info().get_the_id() << std::endl;
            std::cout << fit->vertex(2)->point() << " color: " << fit->vertex(2)->info().get_color()  << " - " << fit->vertex(2)->info().get_the_id() << std::endl;
            std::cout <<"/////////////////////////////////////////////////////////" << std::endl;
            zozela++;
        }
    }

    /*Ipologizetai edw to xrwma pou exei xrwmatisei tis ligoteres korifes etsi wste na topothetithun saftes oi filakes.*/
    int see1 = red.size();
    int see2 = blue.size();
    int see3 = yellow.size();


    int min = see1;
    int selected = 1;

    if(see2 < min){
        min = see2;
        int selected = 2;
    }
    if(see3 < min){
        min = see3;
        int selected = 3;
    }

    if(selected == 1){
        std::cout << "Selected color is red" << std::endl;


    }else if(selected == 2){

         std::cout << "Selected color is blue" << std::endl;

    }else{
         std::cout << "Selected color is yellow" << std::endl;
    }
    std::cout << "Guards must be in the following vertices : " << std::endl;
    if(selected == 1){
        for (int i = 0; i < red.size(); i++) {
                bool get_me_out = false;
            for (CDT::Finite_faces_iterator fit=cdt.finite_faces_begin();
                 fit!=cdt.finite_faces_end();++fit)
            {
                if ( fit->info().in_domain() ){

                    for (int k=0; k < 3; k++) {

                        if (red.at(i) == fit->vertex(k)->info().get_the_id() ) {
                            std::cout << fit->vertex(k)->point() << std::endl;
                            get_me_out = true;
                        }
                    }
                }
                if(get_me_out == true){
                    break;
                }
            }
        }
    }else if(selected == 2){
        for (int i = 0; i< blue.size(); i++) {
             bool get_me_out = false;
            for (CDT::Finite_faces_iterator fit=cdt.finite_faces_begin();
                 fit!=cdt.finite_faces_end();++fit)
            {
                if ( fit->info().in_domain() ){
                    for (int k=0; k < 3; k++) {
                        if (blue.at(i) == fit->vertex(k)->info().get_the_id() ) {
                            std::cout << fit->vertex(k)->point() << std::endl;
                           get_me_out = true;
                        }
                    }
                }
                if(get_me_out == true){
                    break;
                }
            }
        }
    }else if(selected == 3){
        for (int i = 0; i< yellow.size(); i++) {
              bool get_me_out = false;
            for (CDT::Finite_faces_iterator fit=cdt.finite_faces_begin();
                 fit!=cdt.finite_faces_end();++fit)
            {
                if ( fit->info().in_domain() ){
                    for (int k=0; k < 3; k++) {
                        if (yellow.at(i) == fit->vertex(k)->info().get_the_id() ) {
                            std::cout << fit->vertex(k)->point() << std::endl;
                            get_me_out = true;
                        }
                    }
                }
                if(get_me_out == true){
                    break;
                }
            }
        }
    }
    std::cout << "================================================"<< std::endl;
    std::cout << "Each color has the following number of vertices"<< std::endl;
    std::cout << "red " << see1 << std::endl;
    std::cout << "blue " << see2 << std::endl;
    std::cout << "yellow " << see3 << std::endl;
    std::cout << "================================================"<< std::endl;

    std::cout << "There are " <<  facelets_count << " facets in the domain." << std::endl;
      return 0;
}
