#include "interfaces.hpp"


class c_tf2 {
public:
    void updatew2s();
    bool w2s(const vector& origin, vec2& screen);
    void getlocal();
    std::string randomstringfromlist();
    void trace(const vector& start, const vector& end, unsigned int mask, c_trace_filter* pFilter, c_game_trace* pTrace);
    bool computeboundingbox(tf_entity* entity, vector* transformed, rect_t& bbox);
    vec2 getscreensize();
    bool vis_check(tf_player* skip, const tf_player* ent, const vector& from, const vector& to, unsigned int mask);
    void fixmovement(c_user_cmd* cmd, const vector& target_angle);
    static std::wstring ConvertUtf8ToWide(const std::string& str);

    int screenwidth = 0, screenheight = 0;
    D3DMATRIX m_w2s_mat = {};
};

inline c_tf2 tf2 = {};
