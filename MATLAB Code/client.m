function client(port)
%   provides a menu for accessing PIC32 motor control functions
%
%   client_menu(port)
%
%   Input Arguments:
%       port - the name of the com port.  This should be the same as what
%               you use in screen or putty in quotes ' '
%
%   Example:
%       client_menu('/dev/ttyUSB0') (Linux/Mac)
%       client_menu('COM3') (PC)
%
%   For convenience, you may want to embed this in a script that
%   contains your port number
   
% Opening COM connection
if ~isempty(instrfind)
    fclose(instrfind);
    delete(instrfind);
end

fprintf('Opening port %s....\n',port);

% settings for opening the serial port. baud rate 230400, hardware flow control
% wait up to 120 seconds for data before timing out
mySerial = serial(port, 'BaudRate', 230400, 'FlowControl', 'hardware','Timeout',120); 
% opens serial connection
fopen(mySerial);
% closes serial port when function exits
clean = onCleanup(@()fclose(mySerial));                                 

has_quit = false;
% menu loop
while ~has_quit
    % display the menu options
    fprintf('d: Dummy Command          q: Quit \n');
    fprintf('p: reset encoder count    r: read encoder ticks \n');
    fprintf('a: read encoder angle     s: current state \n');
    fprintf('c: current sensor ticks   t: current sensor current (mA) \n');
    fprintf('z: set PWM duty cycle     l: write current gains \n');
    fprintf('g: read current gains     e: recording samples of tuning data \n');
    fprintf('j: reference current mA   y: load trajectory \n');
    fprintf('o: follow loaded traj     x: write position gains \n');
    fprintf('f: read position gains \n\n');

    % read the users choice
    selection = input('Enter Command: ', 's');
     
    % send the command to the PIC32
    fprintf(mySerial,'%c\n',selection);

    
    % take the appropriate action
    switch selection
        case 'd'                              % example operation
            n = input('Enter number: '); % get the number to send
            fprintf(mySerial, '%d\n',n);    % send the number
            n = fscanf(mySerial,'%d');
            fprintf('Read: %d\n',n);
        case 'p'
            fprintf('Encoder Reset!\n');
        case 'r'
            ticks = fscanf(mySerial,'%d');
            fprintf('Encoder Ticks: %d\n', ticks)
        case 'a'
            angle = fscanf(mySerial,'%d');
            fprintf('Encoder Angle: %d\n', angle)
        case 's'
            state = fscanf(mySerial,'%d');
            fprintf('Current State of Utilities: %d\n', state)
        case 'c'
            isense_ticks = fscanf(mySerial,'%d');
            fprintf('Current Sensor Ticks (0-1023): %d\n', isense_ticks)
        case 't'
            isense_amps = fscanf(mySerial,'%d');
            fprintf('Current Sensor Current (mA): %d\n', isense_amps)
        case 'z'
            n = input('Enter PWM duty cycle (-100,100): '); % get the number to send
            fprintf(mySerial, '%d\n',n); 
            OC1RS = fscanf(mySerial,'%d');
            fprintf('Set PWM duty cycle: %d\n', OC1RS/4000)
        case 'l'
            Kp = input('Enter Kp value: '); % get the number to send
            fprintf(mySerial, '%d\n',Kp);
            Ki = input('Enter Ki value: '); % get the number to send
            fprintf(mySerial, '%d\n',Ki);
        case 'g'
            Kp = fscanf(mySerial,'%f');
            Ki = fscanf(mySerial,'%f');
            fprintf('Kp and Ki Gains: %f %f\n', Kp, Ki)
        case 'e'
            read_plot_matrix(mySerial);
        case 'j'
            ref_A = input('Enter reference current (Amps): ');
            fprintf(mySerial, '%d\n',ref_A);
            read_plot_matrix(mySerial);
        case 'y'
            A = input('Enter Trajectory: ');
            traj_type = input('Enter Type of Trajectory ("step" or "cubic"): ');
            if strcmp(traj_type,'step')
                traj_command = 'previous';
            end
            if strcmp(traj_type,'cubic')
                traj_command = 'pchip';
            end
            ref = gen_ref(200, A, traj_command);
            fprintf(mySerial, '%d\n',length(ref));
            i=1;
            while i<=length(ref)
                fprintf(mySerial, '%d\n',ref(i));
                i=i+1;
            end
            traj_len = fscanf(mySerial,'%d');
            fprintf('Length of Trajectory: %d\n', traj_len)
            %read_plot_matrix(mySerial);
        case 'o'
            traj_len = input('Enter trajectory length: ');
            fprintf(mySerial, '%d\n',traj_len);
            additional_samples = input('Enter additional samples to store: ');
            fprintf(mySerial, '%d\n',additional_samples);
            read_plot_matrix(mySerial);
        case 'x'
            Kp = input('Enter Kp value: '); % get the number to send
            fprintf(mySerial, '%d\n',Kp);
            Ki = input('Enter Ki value: '); % get the number to send
            fprintf(mySerial, '%d\n',Ki);
            Kd = input('Enter Kd value: '); % get the number to send
            fprintf(mySerial, '%d\n',Kd);
        case 'f'
            Kp = fscanf(mySerial,'%f');
            Ki = fscanf(mySerial,'%f');
            Kd = fscanf(mySerial,'%f');
            fprintf('Kp, Ki, Kd Gains: %f %f %f\n', Kp, Ki, Kd)
        case 'q'
            has_quit = true;       % exit matlab
        otherwise
            fprintf('Invalid Selection %c\n', selection);
    end
end

end