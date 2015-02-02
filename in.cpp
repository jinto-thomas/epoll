class Employee {
	public:
	    Employee(string theName, float thePayRate);

  	    string getName() const;
  	    float getPayRate() const;

	    float pay(float hoursWorked) const;

	protected:
	  string name;
	  float payRate;
};

Employee::Employee(string theName, float thePayRate)
{
	  name = theName;
	    payRate = thePayRate;
}

string Employee::getName() const
{
	  return name;
}

float Employee::getPayRate() const
{
	  return payRate;
}

float Employee::pay(float hoursWorked) const
{
	  return hoursWorked * payRate;
}
