#include "query_funcs.h"

void drop_all(connection * C) {
  string sql = "DROP TABLE IF EXISTS\"PLAYER\";"
               "DROP TABLE IF EXISTS \"TEAM\";"
               "DROP TABLE IF EXISTS \"STATE\";"
               "DROP TABLE IF EXISTS \"COLOR\";";
  work W(*C);
  W.exec(sql);
  W.commit();
  cout << "all table dropped" << endl;
}

void create_table_player(connection * C) {
  string sql = "CREATE TABLE \"PLAYER\"("
               "\"PLAYER_ID\" SERIAL PRIMARY KEY,"
               "\"TEAM_ID\" INT NOT NULL,"
               "\"UNIFORM_NUM\" INT NOT NULL,"
               "\"FIRST_NAME\" VARCHAR(25) NOT NULL,"
               "\"LAST_NAME\" VARCHAR(25) NOT NULL,"
               "\"MPG\" INT NOT NULL,"              //mins
               "\"PPG\" INT NOT NULL,"              //points
               "\"RPG\" INT NOT NULL,"              //rebounds
               "\"APG\" INT NOT NULL,"              //assists
               "\"SPG\" DECIMAL(3,1) NOT NULL,"     //steals 0.0
               "\"BPG\" DECIMAL(3,1) NOT NULL );";  //blocks 0.0
  work W(*C);
  W.exec(sql);
  W.commit();
}

void add_all_players(connection * C) {
  ifstream player_f("player.txt");
  if (!player_f.is_open()) {
    cerr << "cannot open player.txt" << endl;
    exit(EXIT_FAILURE);
  }
  string line;
  while (getline(player_f, line)) {
    int id, team_id, jersey_num, mpg, ppg, rpg, apg;
    string first_name, last_name;
    double spg, bpg;
    stringstream ss(line);
    ss >> id >> team_id >> jersey_num >> first_name >> last_name >> mpg >> ppg >> rpg >>
        apg >> spg >> bpg;
    // cout << id << " " << team_id << " " << jersey_num << " " << first_name << " "
    //      << last_name << " " << mpg << " " << ppg << " " << rpg << " " << apg << " "
    //      << spg << " " << bpg << endl;
    add_apostrophe(first_name);
    add_apostrophe(last_name);
    add_player(
        C, team_id, jersey_num, first_name, last_name, mpg, ppg, rpg, apg, spg, bpg);
  }
  player_f.close();
}

void add_player(connection * C,
                int team_id,
                int jersey_num,
                string first_name,
                string last_name,
                int mpg,
                int ppg,
                int rpg,
                int apg,
                double spg,
                double bpg) {
  string sql = "INSERT INTO \"PLAYER\" "
               "(\"TEAM_ID\",\"UNIFORM_NUM\",\"FIRST_NAME\",\"LAST_NAME\",\"MPG\","
               "\"PPG\",\"RPG\",\"APG\",\"SPG\",\"BPG\") VALUES (" +
               to_string(team_id) + "," + to_string(jersey_num) + ",\'" + first_name +
               "\',\'" + last_name + "\'," + to_string(mpg) + "," + to_string(ppg) + "," +
               to_string(rpg) + "," + to_string(apg) + "," + to_string(spg) + "," +
               to_string(bpg) + ");";
  work W(*C);
  W.exec(sql);
  W.commit();
}

void create_table_team(connection * C) {
  string sql = "CREATE TABLE \"TEAM\"("
               "\"TEAM_ID\" SERIAL PRIMARY KEY,"
               "\"NAME\" VARCHAR(20) NOT NULL,"
               "\"STATE_ID\" INT NOT NULL,"
               "\"COLOR_ID\" INT NOT NULL,"
               "\"WINS\" INT NOT NULL,"
               "\"LOSSES\" INT NOT NULL);";
  work W(*C);
  W.exec(sql);
  W.commit();
}

void add_all_teams(connection * C) {
  ifstream team_f("team.txt");
  if (!team_f.is_open()) {
    cerr << "cannot open team.txt" << endl;
    exit(EXIT_FAILURE);
  }
  string line;
  while (getline(team_f, line)) {
    int id, state_id, color_id, wins, losses;
    string name;
    stringstream ss(line);
    ss >> id >> name >> state_id >> color_id >> wins >> losses;
    add_apostrophe(name);
    add_team(C, name, state_id, color_id, wins, losses);
  }
  team_f.close();
}

void add_team(connection * C,
              string name,
              int state_id,
              int color_id,
              int wins,
              int losses) {
  string sql = "INSERT INTO \"TEAM\" "
               "(\"NAME\",\"STATE_ID\",\"COLOR_ID\",\"WINS\","
               "\"LOSSES\") VALUES (\'" +
               name + "\'," + to_string(state_id) + "," + to_string(color_id) + "," +
               to_string(wins) + "," + to_string(losses) + ");";
  work W(*C);
  W.exec(sql);
  W.commit();
}

void create_table_state(connection * C) {
  string sql = "CREATE TABLE \"STATE\"("
               "\"STATE_ID\" SERIAL PRIMARY KEY,"
               "\"NAME\" VARCHAR(5) NOT NULL);";
  work W(*C);
  W.exec(sql);
  W.commit();
}

void add_all_states(connection * C) {
  ifstream state_f("state.txt");
  if (!state_f.is_open()) {
    cerr << "cannot open state.txt" << endl;
    exit(EXIT_FAILURE);
  }
  string line;
  while (getline(state_f, line)) {
    int id;
    string name;
    stringstream ss(line);
    ss >> id >> name;
    add_apostrophe(name);
    add_state(C, name);
  }
  state_f.close();
}

void add_state(connection * C, string name) {
  string sql = "INSERT INTO \"STATE\"(\"NAME\") VALUES (\'" + name + "\');";
  work W(*C);
  W.exec(sql);
  W.commit();
}

void create_table_color(connection * C) {
  string sql = "CREATE TABLE \"COLOR\"("
               "\"COLOR_ID\" SERIAL PRIMARY KEY,"
               "\"NAME\" VARCHAR(25) NOT NULL);";
  work W(*C);
  W.exec(sql);
  W.commit();
}

void add_all_colors(connection * C) {
  ifstream color_f("color.txt");
  if (!color_f.is_open()) {
    cerr << "cannot open state.txt" << endl;
    exit(EXIT_FAILURE);
  }
  string line;
  while (getline(color_f, line)) {
    int id;
    string name;
    stringstream ss(line);
    ss >> id >> name;
    add_apostrophe(name);
    add_color(C, name);
  }
  color_f.close();
}

void add_color(connection * C, string name) {
  string sql = "INSERT INTO \"COLOR\"(\"NAME\") VALUES (\'" + name + "\');";
  work W(*C);
  W.exec(sql);
  W.commit();
}

void add_apostrophe(string & str) {
  size_t pos = str.find("'");
  if (pos != string::npos) {
    str.insert(pos, "'");
  }
}
/*
 * All use_ params are used as flags for corresponding attributes
 * a 1 for a use_ param means this attribute is enabled (i.e. a WHERE clause is needed)
 * a 0 for a use_ param means this attribute is disabled
 */
void query1(connection * C,
            int use_mpg,
            int min_mpg,
            int max_mpg,
            int use_ppg,
            int min_ppg,
            int max_ppg,
            int use_rpg,
            int min_rpg,
            int max_rpg,
            int use_apg,
            int min_apg,
            int max_apg,
            int use_spg,
            double min_spg,
            double max_spg,
            int use_bpg,
            double min_bpg,
            double max_bpg) {
}

void query2(connection * C, string team_color) {
}

void query3(connection * C, string team_name) {
}

void query4(connection * C, string team_state, string team_color) {
}

void query5(connection * C, int num_wins) {
}
