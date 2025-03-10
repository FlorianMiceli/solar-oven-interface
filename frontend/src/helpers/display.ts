import { useToast } from "@/shadcn-components/ui/toast";

const { toast } = useToast()

export const success = (title: string, message: string) => {
    toast({
        title: title,
        description: message,
    });
}

export const error = (title: string, message: string) => {
    toast({
        title: title,
        description: message,
    });
}

export const warning = (title: string, message: string) => {
    toast({
        title: title,
        description: message,
    });
}

export const info = (title: string, message: string) => {
    toast({
        title: title,
        description: message,
    });
}