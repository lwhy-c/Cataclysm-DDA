#pragma once
#ifndef PANELS_H
#define PANELS_H

#include "json.h"

#include <functional>
#include <map>
#include <string>
#include <iterator>

class player;
namespace catacurses
{
class window;
} // namespace catacurses
enum face_type : int {
    face_human = 0,
    face_bird,
    face_bear,
    face_cat,
    num_face_types
};

enum sidebar_location {
    top,
    bottom,
    left,
    right
};

// a read-only std::vector<std::string>
class panel_element
{
    public:
        panel_element( std::vector<std::string> input );

        panel_element operator=( std::vector<std::string> rhs );

        bool empty() const;
        std::vector<std::string>::const_iterator cbegin() const;
        std::vector<std::string>::const_iterator cend() const;
        size_t size() const;

    private:
        std::vector<std::string> element_string;
};

// the class that drives each element.
// not to be confused with panel_element, which is a wrapper
class draw_panel_element
{
    public:
        draw_panel_element( std::function<panel_element( int )> func, std::string nm );

        std::string get_id() const;
        std::string get_name() const;
        void print_element( const catacurses::window &w ) const;
        unsigned int get_width() const;
        unsigned int get_height() const;
        void move_element( point to );

    private:
        std::function<panel_element( int )> colored_element_lines;
        point start_location;
        std::string name;
};

class window_panel
{
    public:
        window_panel( std::function<void( player &, const catacurses::window & )> draw_func, std::string nm,
                      int ht, int wd, bool default_toggle );

        std::function<void( player &, const catacurses::window & )> draw;
        int get_height() const;
        int get_width() const;
        std::string get_name() const;
        bool toggle;

        void add_element( draw_panel_element new_element );
        void remove_element( draw_panel_element remove );

    private:
        int height;
        int width;
        bool default_toggle;
        std::string name;
        std::map<std::string, draw_panel_element> panel_elements;
};

class panel_layout
{
    public:
        int index;
        sidebar_location location;
    private:
        std::vector<window_panel> panels;
};

class panel_manager
{
    public:
        panel_manager();
        ~panel_manager() = default;
        panel_manager( panel_manager && ) = default;
        panel_manager( const panel_manager & ) = default;
        panel_manager &operator=( panel_manager && ) = default;
        panel_manager &operator=( const panel_manager & ) = default;

        static panel_manager &get_manager() {
            static panel_manager single_instance;
            return single_instance;
        }

        std::vector<window_panel> &get_current_layout();
        const std::string get_current_layout_id() const;

        void draw_adm( const catacurses::window &w, size_t column = 0, size_t index = 1 );

        void init();

    private:
        bool save();
        bool load();
        void serialize( JsonOut &json );
        void deserialize( JsonIn &jsin );

        std::string current_layout_id;
        std::map<std::string, std::vector<window_panel>> layouts;
        std::map<std::string, draw_panel_element> panel_elements;

};

#endif //PANELS_H
