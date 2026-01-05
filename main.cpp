#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <limits>

using namespace std;

// ------------------- Appointment Base Class -------------------
class Appointment {
protected:
    string date, time, patientID, doctorID;

public:
    Appointment(string d, string t, string pid, string did)
        : date(d), time(t), patientID(pid), doctorID(did) {}

    virtual ~Appointment() = default;

    virtual int getPriority() const = 0;
    virtual string getType() const = 0;

    string getDate() const { return date; }
    string getTime() const { return time; }
    string getPatientID() const { return patientID; }
    string getDoctorID() const { return doctorID; }
};

// ------------------- Regular Appointment -------------------
class RegularAppointment : public Appointment {
    string reason;

public:
    RegularAppointment(string d, string t, string pid, string did, string r)
        : Appointment(d, t, pid, did), reason(r) {}

    int getPriority() const override { return 1; }
    string getType() const override { return "Regular"; }
    string getReason() const { return reason; }
};

// ------------------- Emergency Appointment -------------------
class EmergencyAppointment : public Appointment {
    int urgency;

public:
    EmergencyAppointment(string d, string t, string pid, string did, int u)
        : Appointment(d, t, pid, did), urgency(u) {}

    int getPriority() const override { return urgency; }
    string getType() const override { return "Emergency"; }
};

// ------------------- Patient -------------------
class Patient {
    string name, id;
    vector<Appointment*> appointments;  // non-owning

public:
    Patient(string n, string i) : name(n), id(i) {}

    void addAppointment(Appointment* a) { appointments.push_back(a); }

    void removeAppointment(Appointment* a) {
        appointments.erase(remove(appointments.begin(), appointments.end(), a),
                            appointments.end());
    }

    void viewAppointments() const {
        cout << "\nAppointments for " << name << " (" << id << ")\n";
        if (appointments.empty()) {
            cout << "No appointments.\n";
            return;
        }
        for (auto a : appointments) {
            cout << a->getDate() << " " << a->getTime()
                 << " | Doctor: " << a->getDoctorID()
                 << " | " << a->getType()
                 << " | Priority: " << a->getPriority() << "\n";
        }
    }

    string getID() const { return id; }
};

// ------------------- Doctor -------------------
class Doctor {
    string name, id, specialty;
    vector<Appointment*> appointments;  // non-owning

public:
    Doctor(string n, string i, string s)
        : name(n), id(i), specialty(s) {}

    void addAppointment(Appointment* a) { appointments.push_back(a); }

    void removeAppointment(Appointment* a) {
        appointments.erase(remove(appointments.begin(), appointments.end(), a),
                            appointments.end());
    }

    void viewSchedule() const {
        cout << "\nSchedule for Dr. " << name << " (" << specialty << ")\n";
        if (appointments.empty()) {
            cout << "No appointments.\n";
            return;
        }
        for (auto a : appointments) {
            cout << a->getDate() << " " << a->getTime()
                 << " | Patient: " << a->getPatientID()
                 << " | " << a->getType()
                 << " | Priority: " << a->getPriority() << "\n";
        }
    }

    string getID() const { return id; }
};

// ------------------- Clinic -------------------
class Clinic {
    vector<unique_ptr<Patient>> patients;
    vector<unique_ptr<Doctor>> doctors;
    vector<unique_ptr<Appointment>> appointments;

public:
    Patient* findPatient(const string& id) {
        for (auto& p : patients)
            if (p->getID() == id) return p.get();
        return nullptr;
    }

    Doctor* findDoctor(const string& id) {
        for (auto& d : doctors)
            if (d->getID() == id) return d.get();
        return nullptr;
    }

    void addPatient(string name, string id) {
        patients.push_back(make_unique<Patient>(name, id));
    }

    void addDoctor(string name, string id, string spec) {
        doctors.push_back(make_unique<Doctor>(name, id, spec));
    }

    void bookAppointment(unique_ptr<Appointment> appt) {
        Patient* p = findPatient(appt->getPatientID());
        Doctor* d = findDoctor(appt->getDoctorID());

        if (!p || !d) {
            cout << "Invalid patient or doctor ID.\n";
            return;
        }

        p->addAppointment(appt.get());
        d->addAppointment(appt.get());
        appointments.push_back(move(appt));

        cout << "Appointment booked.\n";
    }

    void cancelAppointment(string pid, string did, string date, string time) {
        auto it = find_if(appointments.begin(), appointments.end(),
            [&](const unique_ptr<Appointment>& a) {
                return a->getPatientID() == pid &&
                       a->getDoctorID() == did &&
                       a->getDate() == date &&
                       a->getTime() == time;
            });

        if (it == appointments.end()) {
            cout << "Appointment not found.\n";
            return;
        }

        Appointment* appt = it->get();
        if (auto p = findPatient(pid)) p->removeAppointment(appt);
        if (auto d = findDoctor(did)) d->removeAppointment(appt);

        appointments.erase(it);
        cout << "Appointment cancelled.\n";
    }

    void generateReport() const {
        vector<Appointment*> sorted;
        for (auto& a : appointments) sorted.push_back(a.get());

        sort(sorted.begin(), sorted.end(),
             [](Appointment* a, Appointment* b) {
                 return a->getPriority() > b->getPriority();
             });

        cout << "\n--- Appointment Report ---\n";
        for (auto a : sorted) {
            cout << a->getDate() << " " << a->getTime()
                 << " | Patient: " << a->getPatientID()
                 << " | Doctor: " << a->getDoctorID()
                 << " | " << a->getType()
                 << " | Priority: " << a->getPriority() << "\n";
        }
    }
};

// ------------------- Helpers -------------------
void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string getString(const string& prompt) {
    string s;
    do {
        cout << prompt;
        getline(cin, s);
    } while (s.empty());
    return s;
}

int getInt(const string& prompt, int min, int max) {
    int x;
    while (true) {
        cout << prompt;
        if (cin >> x && x >= min && x <= max) {
            clearInput();
            return x;
        }
        clearInput();
        cout << "Invalid input.\n";
    }
}

// ------------------- Main -------------------
int main() {
    Clinic clinic;
    int choice;

    do {
        cout << "\n1.Add Patient 2.Add Doctor 3.Book Appointment\n";
        cout << "4.View Report 5.Cancel Appointment 6.Exit\nChoice: ";
        cin >> choice;
        clearInput();

        if (choice == 1) {
            clinic.addPatient(getString("Name: "), getString("Patient ID: "));
        }
        else if (choice == 2) {
            clinic.addDoctor(getString("Name: "), getString("Doctor ID: "),
                             getString("Specialty: "));
        }
        else if (choice == 3) {
            string pid = getString("Patient ID: ");
            string did = getString("Doctor ID: ");
            string date = getString("Date (YYYY-MM-DD): ");
            string time = getString("Time (HH:MM): ");
            string type = getString("Type (Regular/Emergency): ");

            if (type == "Regular") {
                clinic.bookAppointment(make_unique<RegularAppointment>(
                    date, time, pid, did, getString("Reason: ")));
            } else {
                clinic.bookAppointment(make_unique<EmergencyAppointment>(
                    date, time, pid, did, getInt("Urgency (1-5): ", 1, 5)));
            }
        }
        else if (choice == 4) {
            clinic.generateReport();
        }
        else if (choice == 5) {
            clinic.cancelAppointment(
                getString("Patient ID: "),
                getString("Doctor ID: "),
                getString("Date: "),
                getString("Time: "));
        }
    } while (choice != 6);

    return 0;
}
