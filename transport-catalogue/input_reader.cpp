#include "input_reader.h"

using namespace transport_catalogue::input;

void InputReader::CreateRequestList(std::istream& input) {
	
	std::string text;
	std::getline(input, text);
	int count_query = std::stoi(text);
	while (count_query > 0) {
		std::getline(input, text);
		std::string first_str = text.substr(text.find_first_not_of(" "));
		first_str = first_str.substr(0, first_str.find(" "));

		std::string second_str = text.substr(text.find_first_not_of(" "));
		second_str = second_str.substr(second_str.find(" "));
		second_str = second_str.substr(second_str.find_first_not_of(" "));
		second_str = second_str.substr(0, second_str.find_last_not_of(" ") + 1);

		stops_and_buses_.push_back({ first_str, second_str });
		--count_query;
	}
}

void InputReader::AddStops(TransportCatalogue& transport_catalogue) {
	
	for (const auto& c : stops_and_buses_) {
		if (c.first == "Stop") {
			Stop stop;
			std::string stop_name = c.second.substr(c.second.find_first_not_of(" "), c.second.find(":"));
			stop.stop_name = stop_name.substr(0, stop_name.find_last_not_of(" ") + 1);

			std::string str_latitude = c.second.substr(c.second.find(":") + 1);
			str_latitude = str_latitude.substr(str_latitude.find_first_not_of(" "), str_latitude.find(",") - 1);
			stop.latitude = std::stod(str_latitude.substr(0, str_latitude.find_last_not_of(" ") + 1));

			std::string str_longitude = c.second.substr(c.second.find(",") + 1);
			stop.longitude = std::stod(str_longitude.substr(str_longitude.find_first_not_of(" "), str_longitude.find_last_not_of(" ,") + 1));
			
			transport_catalogue.AddStop(stop);
		}
	}
}

void InputReader::AddStopsLength(TransportCatalogue& transport_catalogue) {
	for (const auto& c : stops_and_buses_) {
		if (c.first == "Stop") {
			std::string stop_name = c.second.substr(c.second.find_first_not_of(" "), c.second.find(":"));
			stop_name = stop_name.substr(0, stop_name.find_last_not_of(" ") + 1);

			std::string str = c.second.substr(c.second.find(",") + 1);
			while (str.find(",") != str.npos) {
				str = str.substr(str.find(",") + 1);
				str = str.substr(str.find_first_not_of(" "));
				std::string length = str.substr(0, str.find_first_not_of("0123456789"));
				double length_between = std::stod(length);
				str = str.substr(str.find_first_of(" "));
				str = str.substr(str.find_first_not_of(" "));
				str = str.substr(str.find_first_of(" "));
				str = str.substr(str.find_first_not_of(" "));
				std::string neighbor = str.substr(0, str.find(","));

				transport_catalogue.AddStopLength(&transport_catalogue.FindStop(stop_name)->stop_name, &transport_catalogue.FindStop(neighbor)->stop_name, length_between);

			}
		}
	}

}

void InputReader::AddBusses(TransportCatalogue& transport_catalogue) {
	
	for (const auto& c : stops_and_buses_) {
		if (c.first == "Bus") {
			Bus bus;
			std::string bus_name = c.second.substr(0, c.second.find(":"));
			bus.bus_name = move(bus_name.substr(0, bus_name.find_last_not_of(" ") + 1));
			int64_t pos = c.second.find_first_not_of(" ", c.second.find(":") + 1);
			std::string_view stops_name = c.second;

			if (c.second.find(">") != c.second.npos) {									// ������ ����������� �������
				while (pos != 0) {

					stops_name.remove_prefix(pos);
					std::string_view stop_name = stops_name.substr(0, stops_name.find(" >"));
					bus.route.push_back(transport_catalogue.FindStop(stop_name));
					pos = stops_name.find_first_not_of(" ", stops_name.find(">") + 1);
				}
				transport_catalogue.AddBus(bus);
			}
			else {																		// ������ ��������� �������				
				while (pos != 0) {
					stops_name.remove_prefix(pos);
					std::string_view stop_name = stops_name.substr(0, stops_name.find(" -"));
					bus.route.push_back(transport_catalogue.FindStop(stop_name));
					pos = stops_name.find_first_not_of(" ", stops_name.find("-") + 1);
				}
				auto route_buf = bus.route;
				auto it = ++route_buf.rbegin();
				for (; it != route_buf.rend(); ++it) {
					bus.route.push_back(std::move(*it));
				}
				transport_catalogue.AddBus(bus);
			}
		}
	}
}

InputReader::InputReader(std::istream& input, TransportCatalogue& transport_catalogue) {
	//{LOG_DURATION("CreateRequestList");
	CreateRequestList(input);
	//}
	//std::cout << "CreateRequestList: ok\n";
	//{
	//LOG_DURATION("AddStops");
	AddStops(transport_catalogue);
	//}
	// //{
	//LOG_DURATION("AddStops");
	AddStopsLength(transport_catalogue);
	//}
	//std::cout << "AddStops: ok\n";
	//{LOG_DURATION("AddBusses"); 
	AddBusses(transport_catalogue);
	//}
	//std::cout << "AddBusses: ok\n";
}

