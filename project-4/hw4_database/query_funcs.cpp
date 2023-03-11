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

void add_all_constraints(connection * C) {
  string sql1 =
      "ALTER TABLE \"PLAYER\""
      "ADD CONSTRAINT PFKTID FOREIGN KEY (\"TEAM_ID\") REFERENCES \"TEAM\"(\"TEAM_ID\");";
  string sql2 = "ALTER TABLE \"TEAM\""
                "ADD CONSTRAINT TFKSID FOREIGN KEY (\"STATE_ID\") REFERENCES "
                "\"STATE\"(\"STATE_ID\");"
                "ALTER TABLE \"TEAM\""
                "ADD CONSTRAINT TFKCID FOREIGN KEY (\"COLOR_ID\") REFERENCES "
                "\"COLOR\"(\"COLOR_ID\");";
  work W(*C);
  W.exec(sql1);
  W.exec(sql2);
  W.commit();
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
  vector<string> sqls;
  sqls.push_back("SELECT * FROM \"PLAYER\"");
  if (use_mpg) {
    sqls.push_back("(\"MPG\" BETWEEN " + to_string(min_mpg) + " AND " +
                   to_string(max_mpg) + ")");
  }
  if (use_ppg) {
    sqls.push_back("(\"PPG\" BETWEEN " + to_string(min_ppg) + " AND " +
                   to_string(max_ppg) + ")");
  }
  if (use_rpg) {
    sqls.push_back("(\"RPG\" BETWEEN " + to_string(min_rpg) + " AND " +
                   to_string(max_rpg) + ")");
  }
  if (use_apg) {
    sqls.push_back("(\"APG\" BETWEEN " + to_string(min_apg) + " AND " +
                   to_string(max_apg) + ")");
  }
  if (use_spg) {
    sqls.push_back("(\"SPG\" BETWEEN " + to_string(min_spg) + " AND " +
                   to_string(max_spg) + ")");
  }
  if (use_bpg) {
    sqls.push_back("(\"BPG\" BETWEEN " + to_string(min_bpg) + " AND " +
                   to_string(max_bpg) + ")");
  }
  string sql;
  for (size_t i = 0; i < sqls.size(); i++) {
    if (i == 1) {
      sql += " WHERE ";
    }
    if (i >= 2) {
      sql += " AND ";
    }
    sql += sqls[i];
  }
  sql += ";";
  nontransaction n(*C);
  result R(n.exec(sql));
  cout << "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG"
       << endl;
  cout << fixed << setprecision(1);
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<int>() << " " << c[1].as<int>() << " " << c[2].as<int>() << " "
         << c[3].as<string>() << " " << c[4].as<string>() << " " << c[5].as<int>() << " "
         << c[6].as<int>() << " " << c[7].as<int>() << " " << c[8].as<int>() << " "
         << c[9].as<double>() << " " << c[10].as<double>() << " " << endl;
  }
}

//show the name of each team with the indicated uniform color
void query2(connection * C, string team_color) {
  string sql = "SELECT \"TEAM\".\"NAME\""
               "FROM \"TEAM\", \"COLOR\""
               "WHERE \"TEAM\".\"COLOR_ID\"=\"COLOR\".\"COLOR_ID\""
               " AND \"COLOR\".\"NAME\"=\'" +
               team_color + "\';";
  nontransaction n(*C);
  result R(n.exec(sql));
  cout << "NAME" << endl;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<string>() << endl;
  }
}

//show the first and last name of each player that plays for the indicated team, ordered from highest to lowest ppg (points per game)
void query3(connection * C, string team_name) {
  string sql = "SELECT \"FIRST_NAME\", \"LAST_NAME\""
               "FROM \"PLAYER\",\"TEAM\""
               "WHERE \"PLAYER\".\"TEAM_ID\"=\"TEAM\".\"TEAM_ID\" AND \"NAME\"=\'" +
               team_name + "\' ORDER BY \"PPG\" DESC;";
  nontransaction n(*C);
  result R(n.exec(sql));
  cout << "FIRST_NAME LAST_NAME" << endl;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<string>() << " " << c[1].as<string>() << endl;
  }
}

//show uniform number, first name and last name of each player that plays in the indicated state and wears the indicated uniform color
void query4(connection * C, string team_state, string team_color) {
  string sql = "select \"UNIFORM_NUM\",\"FIRST_NAME\",\"LAST_NAME\""
               "from \"PLAYER\",\"TEAM\",\"STATE\",\"COLOR\""
               "where \"PLAYER\".\"TEAM_ID\"=\"TEAM\".\"TEAM_ID\" AND "
               "\"TEAM\".\"COLOR_ID\"=\"COLOR\".\"COLOR_ID\" AND "
               "\"TEAM\".\"STATE_ID\"=\"STATE\".\"STATE_ID\" AND \"STATE\".\"NAME\"=\'" +
               team_state + "\' AND \"COLOR\".\"NAME\"=\'" + team_color + "\';";
  nontransaction n(*C);
  result R(n.exec(sql));
  cout << "UNIFORM_NUM FIRST_NAME LAST_NAME" << endl;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<int>() << " " << c[1].as<string>() << " " << c[2].as<string>()
         << endl;
  }
}

//show first name and last name of each player, and team name and number of wins for each team that has won more than the indicated number of games
void query5(connection * C, int num_wins) {
  string sql = "select \"FIRST_NAME\", \"LAST_NAME\",\"NAME\",\"WINS\""
               "from \"PLAYER\",\"TEAM\""
               "where \"PLAYER\".\"TEAM_ID\"=\"TEAM\".\"TEAM_ID\" AND \"WINS\" > " +
               to_string(num_wins) + ";";
  nontransaction n(*C);
  result R(n.exec(sql));
  cout << "FIRST_NAME LAST_NAME NAME WINS" << endl;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<string>() << " " << c[1].as<string>() << " " << c[2].as<string>()
         << " " << c[3].as<int>() << endl;
  }
}
