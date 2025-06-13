#include <cassert>
extern "C" {
    #include <wlr/types/wlr_output.h>
    #include <wlr/types/wlr_output_layout.h>
}

class OutputLayout {

};

class Output {
    public:
    
    Output(wlr_output *output): output(output) {}
    
    ~Output() {
        assert(this->output);
        wlr_output_destroy(this->output);
    }

    int x, y;
    float scale;

    private:
    wlr_output *output = nullptr;
};