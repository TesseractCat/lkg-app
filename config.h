struct LKGConfig {
    float pitch;
    float slope;
    float center;
    float viewCone;
    float dpi;
    int invView;

    LKGConfig(std::istream& config_file) {
        config_file.ignore(256, '=');
        config_file >> this->pitch;
        config_file.ignore(256, '=');
        config_file >> this->slope;
        config_file.ignore(256, '=');
        config_file >> this->center;
        config_file.ignore(256, '=');
        config_file >> this->viewCone;
        config_file.ignore(256, '=');
        config_file >> this->invView;
        config_file.ignore(256, '=');
        config_file >> this->dpi;
    }
};
