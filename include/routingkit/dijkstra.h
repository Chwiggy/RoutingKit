#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <routingkit/id_queue.h>
#include <routingkit/point_in_polygon.h>
#include <routingkit/edge_crosses_polygon.h>
#include <routingkit/constants.h>
#include <routingkit/timestamp_flag.h>
#include <vector>

namespace RoutingKit{

class Dijkstra{
public:
	Dijkstra():first_out(nullptr){}

	Dijkstra(const std::vector<unsigned>&first_out, const std::vector<unsigned>&tail, const std::vector<unsigned>&head):
		tentative_distance(first_out.size()-1),
		predecessor_arc(first_out.size()-1),
		was_popped(first_out.size()-1),
		queue(first_out.size()-1),
		first_out(&first_out),
		tail(&tail),
		head(&head){
		assert(!first_out.empty());
		assert(first_out.front() == 0);
		assert(first_out.back() == tail.size());
		assert(first_out.back() == head.size());


	}

	Dijkstra&reset(){
		queue.clear();
		was_popped.reset_all();
        settle_count = 0;
		return *this;
	}

	Dijkstra&reset(const std::vector<unsigned>&first_out, const std::vector<unsigned>&tail, const std::vector<unsigned>&head){
		assert(!first_out.empty());
		assert(first_out.front() == 0);
		assert(first_out.back() == tail.size());
		assert(first_out.back() == head.size());

        this->settle_count = 0;

		if(this->first_out != nullptr && first_out.size() == this->first_out->size()){
			this->first_out = &first_out;
			this->head = &head;
			this->tail = &tail;
			queue.clear();
			was_popped.reset_all();
			return *this;
		}else{
			this->first_out = &first_out;
			this->head = &head;
			this->tail = &tail;
			tentative_distance.resize(first_out.size()-1);
			predecessor_arc.resize(first_out.size()-1);
			was_popped = TimestampFlags(first_out.size()-1);
			queue = MinIDQueue(first_out.size()-1);
			return *this;
		}
	}

	Dijkstra&add_source(unsigned id, unsigned departure_time = 0){
		assert(id < first_out->size()-1);
		tentative_distance[id] = departure_time;
		predecessor_arc[id] = invalid_id;
		queue.push({id, departure_time});
		return *this;
	}

	bool is_finished()const{
		return queue.empty();
	}

	bool was_node_reached(unsigned x)const{
		assert(x < first_out->size()-1);
		return was_popped.is_set(x);
	}

	struct SettleResult{
		unsigned node;
		unsigned distance;
	};

	template<class GetWeightFunc>
	SettleResult settle(const GetWeightFunc&get_weight){
		assert(!is_finished());

		auto p = queue.pop();
		tentative_distance[p.id] = p.key;
		was_popped.set(p.id);

		for(unsigned a=(*first_out)[p.id]; a<(*first_out)[p.id+1]; ++a){
			if(!was_popped.is_set((*head)[a])){
				unsigned w = get_weight(a, p.key);
				if(w < inf_weight){
					if(queue.contains_id((*head)[a])){
						if(queue.decrease_key({(*head)[a], p.key + w})){
							predecessor_arc[(*head)[a]] = a;
						}
					} else {
						queue.push({(*head)[a], p.key + w});
						predecessor_arc[(*head)[a]] = a;
					}
				}
			}
		}
        settle_count++;
		return SettleResult{p.id, p.key};
	}

	unsigned get_distance_to(unsigned x) const {
		assert(x < first_out->size()-1);
		if(was_popped.is_set(x))
			return tentative_distance[x];
		else
			return inf_weight;
	}

	std::vector<unsigned>get_node_path_to(unsigned x) const {
		assert(x < first_out->size()-1);
		std::vector<unsigned>path;
		if(was_node_reached(x)){
			assert(was_node_reached(x));

			unsigned p;
			while(p = predecessor_arc[x], p != invalid_id){
				path.push_back(x);
				x = (*tail)[p];
			}
			path.push_back(x);
			std::reverse(path.begin(), path.end());
		}
		return path;
	}

	std::vector<unsigned>get_arc_path_to(unsigned x) const {
		assert(x < first_out->size()-1);
		std::vector<unsigned>path;
		if(was_node_reached(x)){
			unsigned p;
			while(p = predecessor_arc[x], p != invalid_id){
				path.push_back(p);
				x = (*tail)[p];
			}
			std::reverse(path.begin(), path.end());
		}
		return path;
	}

    unsigned get_settle_count() const {
        return settle_count;
    }


private:
	std::vector<unsigned>tentative_distance;
	std::vector<unsigned>predecessor_arc;

	TimestampFlags was_popped;
	MinIDQueue queue;
    unsigned settle_count = 0;

	const std::vector<unsigned>*first_out;
	const std::vector<unsigned>*tail;
	const std::vector<unsigned>*head;
};

class ScalarGetWeight{
public:
	explicit ScalarGetWeight(const std::vector<unsigned>&weight):weight(&weight){}

	unsigned operator()(unsigned arc, unsigned departure_time)const{
		(void)departure_time;
		return (*weight)[arc];
	}

private:
	const std::vector<unsigned>*weight;
};

class AvoidPolygonsGetWeight{
public:
	explicit AvoidPolygonsGetWeight(
		const std::vector<unsigned>&weight,
		const std::vector<unsigned>&tail,
		const std::vector<unsigned>&head,
		const std::vector<float>&lat,
		const std::vector<float>&lon,
		const std::vector<std::vector<float>>&polys
	): weight(&weight)
	 , tail(&tail)
	 , head(&head)
	 , lat(&lat)
	 , lon(&lon)
	 , polys(&polys)
	{}

	unsigned operator()(unsigned arc, unsigned departure_time) const {
		(void)departure_time;
		for (auto poly : *polys) {
			if ( point_in_polygon((*lat)[(*tail)[arc]], (*lon)[(*tail)[arc]], poly)
				|| point_in_polygon((*lat)[(*head)[arc]], (*lon)[(*head)[arc]], poly)
				|| edge_crosses_polygon((*lat)[(*tail)[arc]],(*lon)[(*tail)[arc]],(*lat)[(*head)[arc]], (*lon)[(*head)[arc]], poly)
			) return inf_weight;
		}
		return (*weight)[arc];
	}

private:
	const std::vector<unsigned>*weight;
	const std::vector<unsigned>*tail;
	const std::vector<unsigned>*head;
	const std::vector<float>*lat;
	const std::vector<float>*lon;
	const std::vector<std::vector<float>>*polys;
	
};
}
#endif
