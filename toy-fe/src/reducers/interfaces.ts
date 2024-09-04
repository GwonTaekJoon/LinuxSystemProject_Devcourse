export type StringObject = {
    [index: string]: string;
};

export interface RobotsQuery {
    page: number;
    id: number | null;
    search: string | null;
    owner: string | null;
    name: string | null;
    status: string | null;
    mode: string | null;
    [key: string]: string | number | null;
}

export interface RobotState {
    id: number;
    hostname: string;
    temperature: string;
}

export interface Robot {
    instance: any;
    state: RobotState;
}

export interface RobotsState {
    initialized: boolean;
    fetching: boolean;
    updating: boolean;
    hideEmpty: boolean;
    gettingQuery: RobotsQuery;
    count: number;
    current: Robot[];
    activities: {
        dumps: {
            [pid: number]: string[];
        };
    };
}

export interface ErrorState {
    message: string;
    reason: string;
    className?: string;
}

export interface NotificationsState {
    errors: {
        robots: {
            fetching: null | ErrorState;
            updating: null | ErrorState;
        };
    };
    messages: {
        robots: {
            cancelingDone: string;
            pausingDone: string;
            resumingDone: string;
        };
    };
}

export interface CombinedState {
    robots: RobotsState;
    notifications: NotificationsState;
}
